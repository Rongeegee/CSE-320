#include "data.h"
#include <string.h>
#include <debug.h>

/*
 * Create a blob with given content and size.
 * The content is copied, rather than shared with the caller.
 * The returned blob has one reference, which becomes the caller's
 * responsibility.
 *
 * @param content  The content of the blob.
 * @param size  The size in bytes of the content.
 * @return  The new blob, which has reference count 1.
 */
BLOB *blob_create(char *content, size_t size){
    BLOB* blob = malloc(sizeof(BLOB));
    pthread_mutex_init(&(blob->mutex),NULL);
    strcpy(blob->content,content);
    strcpy(blob->prefix,content);
    blob->size = size;
    return blob;
}

/*
 * Increase the reference count on a blob.
 *
 * @param bp  The blob.
 * @param why  Short phrase explaining the purpose of the increase.
 * @return  The blob pointer passed as the argument.
 */
BLOB *blob_ref(BLOB *bp, char *why){
    pthread_mutex_lock(&(bp->mutex));
    bp->refcnt++;
    pthread_mutex_unlock(&(bp->mutex));
    debug("%s\n", why);
    return bp;
}

/*
 * Decrease the reference count on a blob.
 * If the reference count reaches zero, the blob is freed.
 *
 * @param bp  The blob.
 * @param why  Short phrase explaining the purpose of the decrease.
 */
void blob_unref(BLOB *bp, char *why){
    pthread_mutex_lock(&(bp->mutex));
    bp->refcnt--;
    pthread_mutex_unlock(&(bp->mutex));
    debug("%s\n", why);
    if(bp->refcnt == 0){
        free(bp);
    }
}

/*
 * Compare two blobs for equality of their content.
 *
 * @param bp1  The first blob.
 * @param bp2  The second blob.
 * @return 0 if the blobs have equal content, nonzero otherwise.
 */
int blob_compare(BLOB *bp1, BLOB *bp2){
    if(strcmp(bp1->content,bp2->content) == 0){
        return 0;
    }
    return 1;
}

/*
 * Hash function for hashing the content of a blob.
 *
 * @param bp  The blob.
 * @return  Hash of the blob.
 */
int blob_hash(BLOB *bp){
    return (int)(*(bp->content));
}

/*
 * Create a key from a blob.
 * The key inherits the caller's reference to the blob.
 *
 * @param bp  The blob.
 * @return  the newly created key.
 */
KEY *key_create(BLOB *bp){
    KEY* key = malloc(sizeof(KEY));
    key->hash = blob_hash(bp);
    key->blob = bp;
    return key;
}

/*
 * Dispose of a key, decreasing the reference count of the contained blob.
 * A key must be disposed of only once and must not be referred to again
 * after it has been disposed.
 *
 * @param kp  The key.
 */
void key_dispose(KEY *kp){
    blob_unref(kp->blob, "dispose key");
    free(kp);
}

/*
 * Compare two keys for equality.
 *
 * @param kp1  The first key.
 * @param kp2  The second key.
 * @return  0 if the keys are equal, otherwise nonzero.
 */
int key_compare(KEY *kp1, KEY *kp2){
    if(kp1->hash != kp2->hash){
        return 1;
    }
    if(kp1->blob->refcnt != kp2->blob->refcnt){
        return 1;
    }
    if(kp1->blob->size != kp2->blob->size){
        return 1;
    }
    if(strcmp(kp1->blob->content,kp2->blob->content) != 0){
        return 1;
    }
    if(strcmp(kp1->blob->prefix,kp2->blob->prefix) != 0){
        return 1;
    }
    return 0;
}

/*
 * Create a version of a blob for a specified creator transaction.
 * The version inherits the caller's reference to the blob.
 * The reference count of the creator transaction is increased to
 * account for the reference that is stored in the version.
 *
 * @param tp  The creator transaction.
 * @param bp  The blob.
 * @return  The newly created version.
 */
VERSION *version_create(TRANSACTION *tp, BLOB *bp){
    VERSION* version = malloc(sizeof(VERSION));
    version->creator = tp;
    tp->refcnt = tp->refcnt + version->blob->refcnt;
    version->blob = bp;
    version->next = NULL;
    version->prev = NULL;
    return version;
}

/*
 * Dispose of a version, decreasing the reference count of the
 * creator transaction and contained blob.  A version must be
 * disposed of only once and must not be referred to again once
 * it has been disposed.
 *
 * @param vp  The version to be disposed.
 */
void version_dispose(VERSION *vp){
    blob_unref(vp->blob, "dispose blob");
    trans_unref(vp->creator, "dispose transaction");
    free(vp);
}