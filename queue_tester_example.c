#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "queue.c"

#define TEST_ASSERT(assert)                             \
do {                                                                    \
        printf("ASSERT: " #assert " ... ");     \
        if (assert) {                                           \
                printf("PASS\n");                               \
        } else  {                                                       \
                printf("FAIL\n");                               \
                exit(1);                                                \
        }                                                                       \
} while(0)

/* Create */
void test_create(void)
{
        fprintf(stderr, "*** TEST create ***\n");

        TEST_ASSERT(queue_create() != NULL);
}

/* Enqueue/Dequeue simple */
void test_queue_simple(void)
{
        int data = 3, *ptr;
        queue_t q;

        fprintf(stderr, "*** TEST queue_simple ***\n");

        q = queue_create();
        queue_enqueue(q, &data);
        queue_dequeue(q, (void**)&ptr);
        TEST_ASSERT(ptr == &data);
}
/*Simple Delete*/
void test_queue_delete(void)
{
    queue_t q;
    int data[] = {1, 2, 3, 4, 5, 42, 6, 7, 8, 9};
    size_t i;

    
    q = queue_create();
    for (i = 0; i < sizeof(data) / sizeof(data[0]); i++)
        queue_enqueue(q, &data[i]);

    
    fprintf(stderr, "*** TEST queue_delete ***\n");
    TEST_ASSERT(queue_delete(q, &data[5]) == 0);
}
/*Delete a data item that's not in the queue*/
void test_queue_delete_notfound(void)
{
    queue_t q;
    int data[] = {1, 2, 3, 4, 5, 42, 6, 7, 8, 9};
    size_t i;

    
    q = queue_create();
    for (i = 0; i < sizeof(data) / sizeof(data[0]); i++)
        queue_enqueue(q, &data[i]);

    int a = 16;
    fprintf(stderr, "*** TEST queue_delete on data that's not in the queue ***\n");
    TEST_ASSERT(queue_delete(q, &a) == -1);
}

static void iterator_inc(queue_t q, void *data)
{
    int *a = (int*)data;

    if (*a == 42)
        queue_delete(q, data);
    else
        *a += 1;
}
/*Simple Iterator*/
void test_iterator(void)
{
    queue_t q;
    int data[] = {1, 2, 3, 4, 5, 42, 6, 7, 8, 9};
    size_t i;

    /* Initialize the queue and enqueue items */
    q = queue_create();
    for (i = 0; i < sizeof(data) / sizeof(data[0]); i++)
        queue_enqueue(q, &data[i]);

    /* Increment every item of the queue, delete item '42' */
    fprintf(stderr, "*** TEST Iterator ***\n");
    queue_iterate(q, iterator_inc);
    TEST_ASSERT(data[0] == 2);
    TEST_ASSERT(queue_length(q) == 9);
}

/*Destroy a non-empty queue*/
void test_queue_destroy(void)
{
    queue_t q;

    fprintf(stderr, "*** TEST queue_destroy ***\n");


    /* Test destroying a non-empty queue */
    q = queue_create();
    int data = 16;
    queue_enqueue(q, &data);
    TEST_ASSERT(queue_destroy(q) == -1);

}
/*Destroy an empty queue*/
void test_queue_destroy_empty(void)
{
        queue_t q;
        fprintf(stderr, "*** TEST queue_destory on empty queue ***\n");
        q = queue_create();
        TEST_ASSERT(queue_destroy(q) == 0);
}
/*Destroy a NULL queue*/
void test_queue_destroy_null(void)
{
        fprintf(stderr, "*** TEST queue_destory on NULL queue ***\n");
        TEST_ASSERT(queue_destroy(NULL) == -1);
}
/*Enqueue NULL data*/
void test_queue_enqueue_nulldata(void)
{
        queue_t q;
        fprintf(stderr, "*** TEST queue_enqueue with NULL data ***\n");
        q = queue_create();
        TEST_ASSERT(queue_enqueue(q, NULL) == -1);
}
/*Enqueue on NULL queue*/
void test_nullqueue_enqueue(void)
{
        int data = 16;
        fprintf(stderr, "*** TEST queue_enqueue on NULL queue ***\n");
        TEST_ASSERT(queue_enqueue(NULL, &data) == -1);
}
/*Dequeue NULL data*/
void test_queue_dequeue_nulldata(void)
{
        queue_t q;
        fprintf(stderr, "*** TEST queue_dequeue with NULL data ***\n");
        q = queue_create();
        TEST_ASSERT(queue_dequeue(q, NULL) == -1);

}
/*Dequeue a NULL queue*/
void test_nullqueue_dequeue(void)
{
        int *ptr;
        fprintf(stderr, "*** TEST queue_dequeue with NULL queue ***\n");
        TEST_ASSERT(queue_dequeue(NULL, (void**)&ptr) == -1);
}
/*Iterate on a NULL queue*/
void test_queue_iterate_null(void)
{
    fprintf(stderr, "*** TEST queue_iterate with NULL Queue ***\n");
    TEST_ASSERT(queue_iterate(NULL, iterator_inc) == -1);
}
int main(void)
{
        test_create();
        test_queue_simple();
        test_iterator();
        test_queue_delete();
        test_queue_delete_notfound();
        test_queue_destroy();
        test_queue_destroy_empty();
        test_queue_destroy_null();
        test_queue_enqueue_nulldata();
        test_queue_dequeue_nulldata();
        test_nullqueue_enqueue();
        test_nullqueue_dequeue();
        test_queue_iterate_null();
        return 0;
}