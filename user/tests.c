#include "stdlib.h"
#include "mem.h"
#include "stdio.h"
#include "syscalls.h"
#include "console.h"
#include "limits.h"

enum test_res_t {
    TEST_PASS = 0,
    TEST_FAIL = 1,
    TEST_SKIP = 2
};


struct test_list {
    enum test_res_t (*f) (void);
    const char * name;
    struct test_list *next;
    struct test_list *last;
};

struct test_list * tests = NULL;

static void add_test (enum test_res_t(*f)(void), const char * const name)
{
    struct test_list *l = mem_alloc(sizeof(struct test_list));
    l->f=f;
    l->next=NULL;
    l->name=name;
    if(tests!=NULL) {
        tests->last->next = l;
        tests->last = l;
    } else {
        tests = l;
        tests->last = tests;
        tests->next = NULL;
    }
}

#define TEST(X) static enum test_res_t X (void)
#define TEST_ADD(X) add_test(X, #X)
static const char * const test_result[] = {
    [TEST_PASS] = "PASS",
    [TEST_FAIL] = "FAIL",
    [TEST_SKIP] = "SKIP"
};

/*****************************************************************************/

/* start / exit / waitpid */

static int test1_procA (void* p)
{
    exit(42);
}

TEST(test_exit_waitpid)
{
    int retval;
    int pid = start(test1_procA, STACK_SIZE, 3, "t1_pA", NULL);
    waitpid(pid, &retval);
    return (retval==42) ? TEST_PASS : TEST_FAIL;
}

/* waitpid on process alive */
static int test1bis_procA (void* p)
{
    usleep(10);
    exit(51);
}

TEST(test_waitpid_alive)
{
    return TEST_SKIP;
    int retval;
    int pid = start(test1bis_procA, STACK_SIZE, 3, "t1b_pA", NULL);
    waitpid(pid, &retval);
    return (retval == 51) ? TEST_PASS : TEST_FAIL;
}

/* start / getpid() */

static int test2_procA (void *p)
{
    exit(getpid());
}

TEST(test_start_getpid)
{
    int retval;
    int pid = start(test2_procA, STACK_SIZE, 3, "t2_pA", NULL);
    waitpid(pid, &retval);
    return (retval==pid) ? TEST_PASS : TEST_FAIL;
}

/* start / getppid() */


static int test3_procA (void *p)
{
    printf("getppid %d\n", getppid());
    exit(getppid());
}

TEST(test_start_getppid)
{
    int retval;
    printf("ppid %d\n", getpid());
    int pid = start(test3_procA, STACK_SIZE, 3, "t3_pA", NULL);
    waitpid(pid, &retval);
    printf("res %d\n", retval);
    return (retval==getpid()) ? TEST_PASS : TEST_FAIL;
}

/* start / waitpid(,NULL) */

static int test4_procA(void*p)
{
    exit(0);
}

TEST(test_waitpid_NULL)
{
    int pid = start(test4_procA, STACK_SIZE, 3, "t4_pA", NULL);
    waitpid(pid, NULL);
    return TEST_PASS;
}

/* start / return instead of exit() */

static int test5_procA(void*p)
{
    return 27;
}

TEST(test_return_noexit)
{
    int retval;
    int pid = start(test5_procA, STACK_SIZE, 3, "t5_pA", NULL);
    waitpid(pid, &retval);
    return (retval==27) ? TEST_PASS : TEST_FAIL;
}



int fils1_1(void * arg) {
    exit(30);
}

int fils1_2(void * arg) {
    exit(31);
}

int fils1_3(void * arg) {
    exit(32);
}

TEST(test_waitpid_neg)
{
    int ret1, ret2, ret3;
    start(fils1_1, STACK_SIZE, 3, "fils1", NULL);
    start(fils1_2, STACK_SIZE, 3, "fils2", NULL);
    start(fils1_3, STACK_SIZE, 3, "fils3", NULL);

    waitpid(-1, &ret1);
    if(ret1 != 30 && ret1 != 31 && ret1 != 32) {
        return TEST_FAIL;
    }

    waitpid(-1, &ret2);
    if(ret2 != 30 && ret2 != 31 && ret2 != 32) {
        return TEST_FAIL;
    }

    waitpid(-1, &ret3);
    if(ret3 != 30 && ret3 != 31 && ret3 != 32) {
        return TEST_FAIL;
    }

    if(ret1 == ret2 || ret1 == ret3 || ret2 == ret3) {
        return TEST_FAIL;
    }

    return TEST_PASS;
}


int fils2_1(void * arg) {
    exit(30);
}

int fils2_2(void * arg) {
    exit(31);
}

int fils2_3(void * arg) {
    exit(32);
}

TEST(test_waitpid_bad)
{
    int ret1, ret2, ret3;
    int pid1 = start(fils2_1, STACK_SIZE, 3, "fils1", NULL);
    int pid2 = start(fils2_2, STACK_SIZE, 3, "fils2", NULL);
    int pid3 = start(fils2_3, STACK_SIZE, 3, "fils3", NULL);

    if(waitpid(15, &ret1) >= 0) {
        return TEST_FAIL;
    }

    if(waitpid(255, &ret1) >= 0) {
        return TEST_FAIL;
    }

    waitpid(pid1, &ret1);
    waitpid(pid2, &ret2);
    waitpid(pid3, &ret3);

    if(waitpid(52, &ret1) >= 0) {
        return TEST_FAIL;
    }

    if(waitpid(155, &ret1) >= 0) {
        return TEST_FAIL;
    }

    return TEST_PASS;
}


int killed1(void * arg) {
    usleep(1000);
    exit(42);
}

TEST(test_kill1) {
    int pid1, rk, rw, ret;

    pid1 = start(killed1, STACK_SIZE, 2, "killed1", NULL);

    rk = kill(pid1);

    rw = waitpid(pid1, &ret);

    if(rk != pid1 || rw != pid1) {
        return TEST_FAIL;
    }

    return TEST_PASS;
}


TEST(test_kill2) {

    if(kill(45)>=0) {
        return TEST_FAIL;
    }

    return TEST_PASS;
}

int killed3(void * arg) {
    exit(42);
}

TEST(test_kill3) {
    int pid1, rk, rw, ret;

    pid1 = start(killed3, STACK_SIZE, 2, "killed1", NULL);

    usleep(1000);

    rk = kill(pid1);

    rw = waitpid(pid1, &ret);

    if(rk >= 0 || rw != pid1) {
        return TEST_FAIL;
    }

    return TEST_PASS;
}

int val;

int killed4_2(void * arg) {
    usleep(500);
    val = getppid();
    printf("val %d\n", val);
    return 43;
}

int killed4_1(void * arg) {
    start(killed4_2, STACK_SIZE, 1, "killed2", NULL);
    return 42;
}

TEST(test_kill4) {
    int pid1, rw, rk, ret;

    val = 0;

    pid1 = start(killed4_1, STACK_SIZE, 3, "killed1", NULL);

    
    rk = kill(pid1);
    rw = waitpid(pid1, &ret);

    if(val != 1) {
        return TEST_FAIL;
    }

    return TEST_PASS;
}


int changeur1(void * arg) {
    usleep(100);
    exit(10);
}
 
TEST(test_getprio) {
    int pid1, ret, prio;
 
    pid1 = start(changeur1, STACK_SIZE, 2, "Changeur1", (void *) 0);

    prio = getprio(pid1);
    
    waitpid(pid1, &ret);

    if(prio != 2) {
        return TEST_FAIL;
    }
 
    return TEST_PASS;
}

int changeur2_1(void * arg) {
    val = 1;
    return 1;
}

int changeur2_2(void * arg) {
    val = 2;
    return 2;
}
 
TEST(test_chprio1) {
    int pid1, pid2, ret;
 
    pid1 = start(changeur2_1, STACK_SIZE, 2, "Changeur1", (void *) 0);
    pid2 = start(changeur2_2, STACK_SIZE, 2, "Changeur2", (void *) 0);

    chprio(pid1, 5);
    
    waitpid(pid1, &ret);
    waitpid(pid2, &ret);

    if(val != 2) {
        return TEST_FAIL;
    }
 
    return TEST_PASS;
}


int changeur3_1(void * arg) {
    val = 1;
    return 1;
}

int changeur3_2(void * arg) {
    val = 2;
    return 2;
}
 
TEST(test_chprio2) {
    int pid1, pid2, ret;
 
    pid1 = start(changeur3_1, STACK_SIZE, 2, "Changeur1", (void *) 0);
    pid2 = start(changeur3_2, STACK_SIZE, 2, "Changeur2", (void *) 0);

    chprio(pid1, 1);
    
    waitpid(pid1, &ret);
    waitpid(pid2, &ret);

    if(val != 1) {
        return TEST_FAIL;
    }
 
    return TEST_PASS;
}


// TEST(test_screate) {
//     int sid1, sid2, sid3, sid4;
// 
//     sid1 = screate(5);
//     if(scount(sid1)!=5 || sid1!=0) {
//         sdelete(sid1);
//         return TEST_FAIL;
//     }
// 
//     sid2 = screate(0);
//     if(scount(sid2)!=0 || sid2!=1) {
//         sdelete(sid1);
//         sdelete(sid2);
//         return TEST_FAIL;
//     }
// 
//     sid3 = screate(2);
//     if(scount(sid3)!=2 || sid3!=2) {
//         sdelete(sid1);
//         sdelete(sid2);
//         sdelete(sid3);
//         return TEST_FAIL;
//     }
// 
//     if(screate(-1) >= 0) {
//         sdelete(sid1);
//         sdelete(sid2);
//         sdelete(sid3);
//         return TEST_FAIL;
//     }
// 
//     sid4 = screate(20);
//     if(scount(sid4)!=20 || sid4!=3) {
//         sdelete(sid1);
//         sdelete(sid2);
//         sdelete(sid3);
//         sdelete(sid4);
//         return TEST_FAIL;
//     }
// 
//     sdelete(sid1);
//     sdelete(sid2);
//     sdelete(sid3);
//     sdelete(sid4);
// 
//     return TEST_PASS;
// }
// 
// TEST(test_sdelete1) {
//     int sid1, sid2, sid3, sid4;
// 
//     sid1 = screate(5);
//     sid2 = screate(0);
//     sid3 = screate(2);
//     sid4 = screate(20);
// 
//     if(sdelete(sid1)<0) {
//         return TEST_FAIL;
//     }
// 
//     sid1 = screate(4);
//     if(sid1 != 0 && scount(sid1)!=4) {
//         return TEST_FAIL;
//     }
// 
//     if(sdelete(sid1)<0) {
//         return TEST_FAIL;
//     }
// 
//     if(sdelete(sid1)>=0) {
//         return TEST_FAIL;
//     }
// 
//     if(sdelete(sid2)<0) {
//         return TEST_FAIL;
//     }
// 
//     if(sdelete(sid3)<0) {
//         return TEST_FAIL;
//     }
// 
//     if(sdelete(sid4)<0) {
//         return TEST_FAIL;
//     }
// 
//     if(sdelete(sid4)>=0) {
//         return TEST_FAIL;
//     }
// 
//     sid1 = screate(25);
//     sid2 = screate(26);
//     sid3 = screate(27);
//     sid4 = screate(28);
// 
//     if(scount(sid1)!=25 || sid1!=0
//         || scount(sid2)!=26 || sid2!=1
//         || scount(sid3)!=27 || sid3!=2
//         || scount(sid4)!=28 || sid4!=3) {
//         sdelete(sid1);
//         sdelete(sid2);
//         sdelete(sid3);
//         sdelete(sid4);
//         return TEST_FAIL;
//     }
// 
//     sdelete(sid1);
//     sdelete(sid2);
//     sdelete(sid3);
//     sdelete(sid4);
// 
//     return TEST_PASS;
// }
// 
// int sid;
// 
// int waiter1(void * arg) {
//     wait(sid);
//     val = 2;
//     return val;
// }
// 
// int signaler1(void * arg) {
//     val = 1;
//     return signal(sid);
// }
// 
// TEST(test_wait_signal1) {
//     int pid1, pid2, ret;
// 
//     sid = screate(0);
// 
//     val = 0;
// 
//     pid1 = start(waiter1, STACK_SIZE, 5, "Waiter1", (void *) 0);
//     pid2 = start(signaler1, STACK_SIZE, 5, "Signaler1", (void *) 0);
// 
//     waitpid(pid1, &ret);
//     waitpid(pid2, &ret);
// 
//     if(scount(sid)!=0) {
//         sdelete(sid);
//         return TEST_FAIL;
//     }
// 
//     sdelete(sid);
// 
//     if(val == 2) {
//         return TEST_PASS;
//     }
//     else {
//         return TEST_FAIL;
//     }
// }
// 
// int waiter2(void * arg) {
//     wait(sid);
//     val = 2;
//     return val;
// }
// 
// int signaler2(void * arg) {
//     signal(sid);
//     val = 1;
//     return val;
// }
// 
// TEST(test_wait_signal2) {
//     int pid1, pid2, ret;
// 
//     sid = screate(1);
// 
//     val = 0;
// 
//     pid1 = start(waiter2, STACK_SIZE, 6, "Waiter1", (void *) 0);
//     pid2 = start(signaler2, STACK_SIZE, 5, "Signaler1", (void *) 0);
// 
//     waitpid(pid1, &ret);
//     waitpid(pid2, &ret);
// 
//     sdelete(sid);
// 
//     if(val == 1) {
//         return TEST_PASS;
//     }
//     else {
//         return TEST_FAIL;
//     }
// }
// 
// int waiter3_1(void * arg) {
//     wait(sid);
//     val = 2;
//     return val;
// }
// 
// int waiter3_2(void * arg) {
//     wait(sid);
//     val = 3;
//     return val;
// }
// 
// int signaler3(void * arg) {
//     val = 1;
//     return signal(sid);
// }
// 
// TEST(test_wait_signal3) {
//     int pid1, pid2, pid3, ret;
//     short int cpt;
// 
//     sid = screate(0);
// 
//     val = 0;
// 
//     pid1 = start(waiter3_1, STACK_SIZE, 7, "Waiter1", (void *) 0);
//     pid2 = start(waiter3_2, STACK_SIZE, 6, "Waiter2", (void *) 0);
//     pid3 = start(signaler3, STACK_SIZE, 5, "Signaler1", (void *) 0);
// 
//     waitpid(pid1, &ret);
//     waitpid(pid3, &ret);
// 
//     kill(pid2);
// 
//     waitpid(pid2, &ret);
// 
//     cpt = scount(sid);
// 
//     if(cpt!=255) {
//         sdelete(sid);
//         return TEST_FAIL;
//     }
// 
//     sdelete(sid);
//     
//     if(val == 2) {
//         return TEST_PASS;
//     }
//     else {
//         return TEST_FAIL;
//     }
// }
// 
// int waiter4_1(void * arg) {
//     wait(sid);
//     val += 1;
//     return val;
// }
// 
// int waiter4_2(void * arg) {
//     wait(sid);
//     val += 1;
//     return val;
// }
// 
// int signaler4(void * arg) {
//     signal(sid);
//     return signal(sid);
// }
// 
// TEST(test_wait_signal4) {
//     int pid1, pid2, pid3, ret;
// 
//     sid = screate(0);
// 
//     val = 0;
// 
//     pid1 = start(waiter4_1, STACK_SIZE, 7, "Waiter1", (void *) 0);
//     pid2 = start(waiter4_2, STACK_SIZE, 6, "Waiter2", (void *) 0);
//     pid3 = start(signaler4, STACK_SIZE, 5, "Signaler1", (void *) 0);
// 
//     waitpid(pid1, &ret);
//     waitpid(pid2, &ret);
//     waitpid(pid3, &ret);
// 
//     if(scount(sid) != 0) {
//         sdelete(sid);
//         return TEST_PASS;
//     }
// 
//     sdelete(sid);
// 
//     if(val == 2) {
//         return TEST_PASS;
//     }
//     else {
//         return TEST_FAIL;
//     }
// }
// 
// int waiter5_1(void * arg) {
//     return wait(sid);
// }
// 
// int waiter5_2(void * arg) {
//     return wait(sid);
// }
// 
// int signaler5(void * arg) {
//     return sreset(sid, 1);
// }
// 
// TEST(test_sreset1) {
//     int pid1, pid2, pid3, ret1, ret2, ret3;
// 
//     sid = screate(0);
// 
//     pid1 = start(waiter5_1, STACK_SIZE, 7, "Waiter1", (void *) 0);
//     pid2 = start(waiter5_2, STACK_SIZE, 7, "Waiter2", (void *) 0);
//     pid3 = start(signaler5, STACK_SIZE, 5, "Signaler1", (void *) 0);
// 
//     waitpid(pid1, &ret1);
//     waitpid(pid2, &ret2);
//     waitpid(pid3, &ret3);
// 
//     if(ret1 != -4 || ret2 != -4 || scount(sid)!= 1) {
//         sdelete(sid);
//         return TEST_FAIL;
//     }
// 
//     sdelete(sid);
// 
//     return TEST_PASS;
// }
// 
// int waiter6_1(void * arg) {
//     wait(sid);
//     return wait(sid);
// }
// 
// int waiter6_2(void * arg) {
//     wait(sid);
//     return wait(sid);
// }
// 
// int signaler6(void * arg) {
//     sreset(sid, 0);
//     return signal(sid);
// }
// 
// TEST(test_sreset2) {
//     int pid1, pid2, pid3, ret, ret2, ret3;
// 
//     sid = screate(0);
// 
//     pid1 = start(waiter6_1, STACK_SIZE, 7, "Waiter1", (void *) 0);
//     pid2 = start(waiter6_2, STACK_SIZE, 5, "Waiter2", (void *) 0);
//     pid3 = start(signaler6, STACK_SIZE, 5, "Signaler1", (void *) 0);
// 
//     waitpid(pid1, &ret);
//     waitpid(pid3, &ret3);
// 
//     sreset(sid, 5);
//     waitpid(pid2, &ret2);
// 
//     if(ret2 != -4 || scount(sid)!= 5) {
//         sdelete(sid);
//         return TEST_FAIL;
//     }
// 
//     sdelete(sid);
// 
//     return TEST_PASS;
// }
// 
// int waiter7_1(void * arg) {
//     return wait(sid);
// }
// 
// int waiter7_2(void * arg) {
//     return wait(sid);
// }
// 
// int signaler7(void * arg) {
//     return sdelete(sid);
// }
// 
// TEST(test_sdelete2) {
//     int pid1, pid2, pid3, ret1, ret2, ret3;
// 
//     sid = screate(0);
// 
//     pid1 = start(waiter7_1, STACK_SIZE, 7, "Waiter1", (void *) 0);
//     pid2 = start(waiter7_2, STACK_SIZE, 5, "Waiter2", (void *) 0);
//     pid3 = start(signaler7, STACK_SIZE, 5, "Signaler1", (void *) 0);
// 
//     waitpid(pid1, &ret1);
//     waitpid(pid2, &ret2);
//     waitpid(pid3, &ret3);
// 
//     if(ret2 != -3 || ret1 != -3) {
//         printf("ret\n");
//         return TEST_FAIL;
//     }
// 
//     sid = screate(5);
//     if(sid != 0) {
//         printf("sid\n");
//         sdelete(sid);
//         return TEST_FAIL;
//     }
// 
//     sdelete(sid);
// 
//     return TEST_PASS;
// }
// 
// int waiter8_1(void * arg) {
//     return wait(sid);
// }
// 
// int waiter8_2(void * arg) {
//     return wait(sid);
// }
// 
// int signaler8(void * arg) {
//     sreset(sid, 0);
//     sdelete(sid);
//     sid = screate(0);
//     return sid;
// }
// 
// TEST(test_sdelete3) {
//     int pid1, pid2, pid3, ret1, ret2, ret3;
// 
//     sid = screate(0);
// 
//     pid1 = start(waiter8_1, STACK_SIZE, 9, "Waiter1", (void *) 0);
//     pid2 = start(waiter8_2, STACK_SIZE, 9, "Waiter2", (void *) 0);
//     pid3 = start(signaler8, STACK_SIZE, 5, "Signaler1", (void *) 0);
// 
//     waitpid(pid1, &ret1);
//     waitpid(pid2, &ret2);
//     waitpid(pid3, &ret3);
// 
//     if(ret2 != -4 || ret1 != -4 || scount(sid) != 0) {
//         return TEST_FAIL;
//     }
// 
//     sdelete(sid);
// 
//     return TEST_PASS;
// }
// 
// int waiter9_1(void * arg) {
//     return wait(sid);
// }
// 
// int waiter9_2(void * arg) {
//     return wait(sid);
// }
// 
// int waiter9_3(void * arg) {
//     return wait(sid);
// }
// 
// int signaler9(void * arg) {
//     return signaln(sid, 3);
// }
// 
// TEST(test_signaln1) {
//     int pid1, pid2, pid3, pid4, ret1, ret2, ret3, ret4;
// 
//     sid = screate(0);
// 
//     pid1 = start(waiter9_1, STACK_SIZE, 5, "Waiter1", (void *) 0);
//     pid2 = start(waiter9_2, STACK_SIZE, 5, "Waiter2", (void *) 0);
//     pid3 = start(waiter9_3, STACK_SIZE, 5, "Waiter3", (void *) 0);
//     pid4 = start(signaler9, STACK_SIZE, 5, "Signaler1", (void *) 0);
// 
//     waitpid(pid1, &ret1);
//     waitpid(pid2, &ret2);
//     waitpid(pid3, &ret3);
//     waitpid(pid4, &ret4);
// 
//     if(kill(pid1) >= 0 || kill(pid2) >= 0 || kill(pid3) >= 0) {
//         sdelete(sid);
//         return TEST_FAIL;
//     }
// 
//     if(ret2 != 0 || ret1 != 0 || scount(sid) != 0) {
//         sdelete(sid);
//         return TEST_FAIL;
//     }
// 
//     sdelete(sid);
// 
//     return TEST_PASS;
// }
// 
// int waiter10_1(void * arg) {
//     return wait(sid);
// }
// 
// int waiter10_2(void * arg) {
//     return wait(sid);
// }
// 
// int waiter10_3(void * arg) {
//     return wait(sid);
// }
// 
// int signaler10(void * arg) {
//     return signaln(sid, 2);
// }
// 
// TEST(test_signaln2) {
//     int pid1, pid2, pid3, pid4, ret1, ret2, ret3, ret4;
// 
//     sid = screate(0);
// 
//     pid1 = start(waiter10_1, STACK_SIZE, 9, "Waiter1", (void *) 0);
//     pid2 = start(waiter10_2, STACK_SIZE, 8, "Waiter2", (void *) 0);
//     pid3 = start(waiter10_3, STACK_SIZE, 7, "Waiter3", (void *) 0);
//     pid4 = start(signaler10, STACK_SIZE, 5, "Signaler1", (void *) 0);
// 
//     waitpid(pid1, &ret1);
//     waitpid(pid2, &ret2);
//     waitpid(pid4, &ret4);
// 
//     if(kill(pid1) >= 0 || kill(pid2) >= 0 || kill(pid3) < 0) {
//         sdelete(sid);
//         waitpid(pid3, &ret3);
//         return TEST_FAIL;
//     }
// 
//     waitpid(pid3, &ret3);
// 
//     if(ret2 != 0 || ret1 != 0) {
//         sdelete(sid);
//         return TEST_FAIL;
//     }
// 
//     sdelete(sid);
// 
//     return TEST_PASS;
// }
// 
// int waiter11(void * arg) {
//     return try_wait(sid);
// }
// 
// 
// TEST(test_trywait1) {
//     int pid1, ret1;
// 
//     sid = screate(0);
// 
//     pid1 = start(waiter11, STACK_SIZE, 9, "Waiter1", (void *) 0);
// 
//     waitpid(pid1, &ret1);
// 
//     if(scount(sid) != 0) {
//         sdelete(sid);
//         return TEST_FAIL;
//     }
// 
//     sdelete(sid);
// 
//     return TEST_PASS;
// }
// 
// 
// int waiter12(void * arg) {
//     return try_wait(sid);
// }
// 
// 
// TEST(test_trywait2) {
//     int pid1, ret1;
// 
//     sid = screate(1);
// 
//     pid1 = start(waiter12, STACK_SIZE, 6, "Waiter1", (void *) 0);
// 
//     waitpid(pid1, &ret1);
// 
//     if(scount(sid) != 0) {
//         sdelete(sid);
//         return TEST_FAIL;
//     }
// 
//     sdelete(sid);
// 
//     return TEST_PASS;
// }
// 
// int signaler13(void * arg) {
//     return signaln(sid, SHRT_MAX);
// }
// 
// 
// TEST(test_signaln3) {
//     int pid1, ret1;
// 
//     sid = screate(1);
// 
//     pid1 = start(signaler13, STACK_SIZE, 6, "Waiter1", (void *) 0);
// 
//     waitpid(pid1, &ret1);
// 
//     if(ret1 != -2) {
//         sdelete(sid);
//         return TEST_FAIL;
//     }
// 
//     sdelete(sid);
// 
//     return TEST_PASS;
// }
// 
// TEST(vfs_read)
// {
//     int fd = open("/dev/answer");
//     if(fd<0)
//         return TEST_FAIL;
//     char buf[30];
//     if(read(fd, buf, 30) != 30)
//         return TEST_FAIL;
//     int i;
//     for(i=0;i<30;i++) {
//         if(buf[i] != 42)
//             return TEST_FAIL;
//     }
//     if(close(fd) != 0)
//         return TEST_FAIL;
//     return TEST_PASS;
// }
// 
// TEST(vfs_dev_zero)
// {
//     int fd = open("/dev/zero");
//     if(fd<0)
//         return TEST_FAIL;
//     char buf[30];
//     if(read(fd, buf, 30) != 30)
//         return TEST_FAIL;
//     int i;
//     for(i=0;i<30;i++) {
//         if(buf[i] != 0)
//             return TEST_FAIL;
//     }
//     if(close(fd) != 0)
//         return TEST_FAIL;
//     return TEST_PASS;
// }
// 
// TEST(vfs_dev_null)
// {
//     int fd = open("/dev/null");
//     if(fd<0)
//         return TEST_FAIL;
//     char buf[30];
//     if(read(fd, buf, 30) != 0)
//         return TEST_FAIL;
//     if(close(fd) != 0)
//         return TEST_FAIL;
//     return TEST_PASS;
// }

/*****************************************************************************/

void run_test_suite(void)
{
    /***TEST_ADD()s***/

    // WAITPID, EXIT
    TEST_ADD(test_exit_waitpid);
    TEST_ADD(test_waitpid_alive);
    TEST_ADD(test_waitpid_NULL);
    TEST_ADD(test_return_noexit);
    TEST_ADD(test_waitpid_neg);
    TEST_ADD(test_waitpid_bad);

    // GETPID, GETPPID
    TEST_ADD(test_start_getpid);
    TEST_ADD(test_start_getppid);

    // KILL
    TEST_ADD(test_kill1);
    TEST_ADD(test_kill2);
    TEST_ADD(test_kill3);
    TEST_ADD(test_kill4);

    // GETPRIO, CHPRIO
    TEST_ADD(test_getprio);
    TEST_ADD(test_chprio1);
    TEST_ADD(test_chprio2);


    // SEMAPHORES
//     TEST_ADD(test_screate);
//     TEST_ADD(test_sdelete1);
//     TEST_ADD(test_wait_signal1);
//     TEST_ADD(test_wait_signal2);
//     TEST_ADD(test_wait_signal3);
//     TEST_ADD(test_wait_signal4);
//     TEST_ADD(test_sreset1);
//     TEST_ADD(test_sreset2);
//     TEST_ADD(test_sdelete2);
//     TEST_ADD(test_sdelete3);
//     TEST_ADD(test_signaln1);
//     TEST_ADD(test_signaln2);
//     TEST_ADD(test_trywait1);
//     TEST_ADD(test_trywait2);
//     TEST_ADD(test_signaln3);

    // VFS
//     TEST_ADD(vfs_read);
//     TEST_ADD(vfs_dev_zero);
//     TEST_ADD(vfs_dev_null);

    // SIGNAUX

    /*****************/
    printf("Running test suite\n");
    struct test_list * l = tests;
    int i = 1, tests_run=0, tests_total[]={
        [TEST_PASS] = 0,
        [TEST_FAIL] = 0,
        [TEST_SKIP] = 0
    };
    static int test_colors[] = {
        [TEST_PASS] = 0x02,
        [TEST_FAIL] = 0x84,
        [TEST_SKIP] = 0x05
    };

    while(l) {
        enum test_res_t retval = l->f();
        printf ("%-3d - %-66s ", i, l->name);
        cons_ioctl(CONS_SETATTR, &test_colors[retval]);
        printf("[%s]\n",test_result[retval]);
        int attr = 0x07;
        cons_ioctl(CONS_SETATTR, &attr);

        tests_run++;
        tests_total[retval]++;
        struct test_list * free_me = l;
        l = l->next;
        mem_free(free_me, sizeof(struct test_list));
        i++;
    }
    printf( "Summary :\n"
            "%d/%d ok\n"
            "%d/%d failed\n"
            "%d/%d skipped\n",
            tests_total[TEST_PASS], tests_run,
            tests_total[TEST_FAIL], tests_run,
            tests_total[TEST_SKIP], tests_run
    );
}
