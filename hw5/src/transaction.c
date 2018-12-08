#include "transaction.h"
#include <semaphore.h>
#include <pthread.h>
#include <stdlib.h>
#include "debug.h"
#include "csapp.h"

int largestID = 0;

/*
 * Initialize the transaction manager.
 */
void trans_init(void){
    trans_list.id = -1;
    trans_list.refcnt = 0;
    trans_list.status = TRANS_PENDING;
    trans_list.depends = NULL;
    trans_list.waitcnt = 0;
    sem_init(&trans_list.sem,0,1);
    pthread_mutex_init(&(trans_list.mutex),NULL);
    trans_list.next = &trans_list;
    trans_list.prev = &trans_list;
}

/*
 * Finalize the transaction manager.
 */
void trans_fini(void){
    TRANSACTION* transaction = trans_list.next;
    while(transaction != &trans_list){
        if(transaction->next == &trans_list){
            free(transaction);
            break;
        }
        else{
            TRANSACTION* temp_trans = transaction;
            transaction = transaction->next;
            free(temp_trans);
        }

    }
}

/*
 * Create a new transaction.
 *
 * @return  A pointer to the new transaction (with reference count 1)
 * is returned if creation is successful, otherwise NULL is returned.
 */
TRANSACTION *trans_create(void){
    TRANSACTION* transaction = malloc(sizeof(TRANSACTION));
    transaction->id = largestID;
    transaction->refcnt = 0;
    transaction->status = TRANS_PENDING;
    transaction->waitcnt = 0;
    sem_init(&transaction->sem,0,1);
    pthread_mutex_init(&(transaction->mutex),NULL);
    TRANSACTION* lastTrans = trans_list.prev;
    transaction->next = &trans_list;
    transaction->prev = lastTrans;
    lastTrans->next = transaction;
    largestID++;
    return transaction;
}


/*
 * Increase the reference count on a transaction.
 *
 * @param tp  The transaction.
 * @param why  Short phrase explaining the purpose of the increase.
 * @return  The transaction pointer passed as the argument.
 */
TRANSACTION *trans_ref(TRANSACTION *tp, char *why){
    pthread_mutex_lock(&(tp->mutex));
    tp->refcnt++;
    debug("%s\n",why);
    pthread_mutex_unlock(&(tp->mutex));
    return tp;
}

/*
 * Decrease the reference count on a transaction.
 * If the reference count reaches zero, the transaction is freed.
 *
 * @param tp  The transaction.
 * @param why  Short phrase explaining the purpose of the decrease.
 */
void trans_unref(TRANSACTION *tp, char *why){
    if(tp == NULL){
        return;
    }
    pthread_mutex_lock(&(tp->mutex));
    if(tp->refcnt > 0){
        tp->refcnt--;
    }
    debug("%s\n", why);
    if(tp->refcnt == 0){
        DEPENDENCY *depend = tp->depends;
        while(depend != NULL){
            if(depend->next == NULL){
                free(depend);
                break;
            }
            else{
                DEPENDENCY* tempDep = depend;
                depend = depend->next;
                free(tempDep);
            }
        }
        free(tp);
        return;
    }
    pthread_mutex_unlock(&(tp->mutex));
}

/*
 * Add a transaction to the dependency set for this transaction.
 *
 * @param tp  The transaction to which the dependency is being added.
 * @param dtp  The transaction that is being added to the dependency set.
 */
void trans_add_dependency(TRANSACTION *tp, TRANSACTION *dtp){
    if(tp->depends == NULL){
        DEPENDENCY* dependency = malloc(sizeof(DEPENDENCY));
        dependency->trans = dtp;
        dependency->next = NULL;
    }
    else{
        DEPENDENCY* dependency = tp->depends;
        while(dependency->next != NULL){
            dependency = dependency->next;
        }
        DEPENDENCY* newDependency = malloc(sizeof(DEPENDENCY));
        newDependency->trans = dtp;
        newDependency->next = NULL;
        dependency->next = newDependency;
    }
}

/* Try to commit a transaction.  Committing a transaction requires waiting
 * for all transactions in its dependency set to either commit or abort.
 * If any transaction in the dependency set abort, then the dependent
 * transaction must also abort.  If all transactions in the dependency set
 * commit, then the dependent transaction may also commit.
 *
 * In all cases, this function consumes a single reference to the transaction
 * object.
 *
 * @param tp  The transaction to be committed.
 * @return  The final status of the transaction: either TRANS_ABORTED,
 * or TRANS_COMMITTED.
*/
TRANS_STATUS trans_commit(TRANSACTION *tp){
    pthread_mutex_lock(&(tp->mutex));
    if(tp->depends == NULL){
        tp->status = TRANS_COMMITTED;
        return tp->status;
    }
    DEPENDENCY* dependency = tp->depends;
    while(dependency != NULL){
        if(dependency->trans->status == TRANS_PENDING){
            pthread_mutex_lock(&(dependency->trans->mutex));
            dependency->trans->waitcnt++;
            pthread_mutex_unlock(&(dependency->trans->mutex));
            P(&dependency->trans->sem);
        }
    }
    int waitNumber = tp->waitcnt;
    for(int i = 0; i < waitNumber; i++){
            V(&tp->sem);
            tp->waitcnt--;
    }
    tp->status = TRANS_COMMITTED;
    pthread_mutex_unlock(&(tp->mutex));
    return tp->status;
}

/*
 * Abort a transaction.  If the transaction has already committed, it is
 * a fatal error and the program crashes.  If the transaction has already
 * aborted, no change is made to its state.  If the transaction is pending,
 * then it is set to the aborted state, and any transactions dependent on
 * this transaction must also abort.
 *
 * In all cases, this function consumes a single reference to the transaction
 * object.
 *
 * @param tp  The transaction to be aborted.
 * @return  TRANS_ABORTED.
 */
TRANS_STATUS trans_abort(TRANSACTION *tp){
    if(tp->status == TRANS_COMMITTED){
        abort();
    }
    else if(tp->status == TRANS_ABORTED){
        return tp->status;
    }
    else if(tp->status == TRANS_PENDING){
        tp->status = TRANS_ABORTED;
        pthread_mutex_lock(&(tp->mutex));
        int waitNumber = tp->waitcnt;
        for(int i = 0; i < waitNumber; i++){
                V(&tp->sem);
                tp->waitcnt--;
        }
        trans_unref(tp,"abort status");
        pthread_mutex_unlock(&(tp->mutex));
    }
    return TRANS_ABORTED;
}


/*
 * Get the current status of a transaction.
 * If the value returned is TRANS_PENDING, then we learn nothing,
 * because unless we are holding the transaction mutex the transaction
 * could be aborted at any time.  However, if the value returned is
 * either TRANS_COMMITTED or TRANS_ABORTED, then that value is the
 * stable final status of the transaction.
 *
 * @param tp  The transaction.
 * @return  The status of the transaction, as it was at the time of call.
 */
TRANS_STATUS trans_get_status(TRANSACTION *tp){
    return tp->status;
}


/*
 * Print information about a transaction to stderr.
 * No locking is performed, so this is not thread-safe.
 * This should only be used for debugging.
 *
 * @param tp  The transaction to be shown.
 */
void trans_show(TRANSACTION *tp){
    fprintf(stderr, "Transaction ID is %d\n", tp->status);
    fprintf(stderr, "Transaction reference count is %d\n", tp->refcnt);
    fprintf(stderr, "Transaction status is %d\n", tp->status);
}

/*
 * Print information about all transactions to stderr.
 * No locking is performed, so this is not thread-safe.
 * This should only be used for debugging.
 */
void trans_show_all(void){
    trans_show(&trans_list);
    TRANSACTION* transaction = trans_list.next;
    while(transaction != &trans_list){
        trans_show(transaction);
        transaction = transaction->next;
    }
}