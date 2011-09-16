#include "signals.h"
#include "debug.h"
//*********************************Max
void sigaction(int signalid, void* ad_trt){
    assert(signalid>=1 && signalid<=NBSIG);
    tab_proc[pid_cour-1].sigmap[signalid]=ad_trt;
    return;
}

void signal_send(int pid, int signalid){
    assert(tab_proc[pid-1].valide==1);
    assert(signalid>=1 && signalid<=NBSIG);
    list_put_first (tab_proc[pid-1].sigs,signalid);
    printf("    on vient d'envoyer du signal \n");
    return;
}

void trt_sigs(void){
    int signalid;
    void *trt_sig;
    size_t nb_elem= STACK_SIZE / sizeof(unsigned long);
    if (pid_cour==0) return;
    if (!list_is_empty(tab_proc[pid_cour-1].sigs) && tab_proc[pid_cour-1].cur_sig==0){
        tab_proc[pid_cour-1].cur_sig=1;
        printf("            trt_sigs\n");
        tab_proc[pid_cour-1].edi       = ( tab_proc[pid_cour-1].kernel_stack[nb_elem-14] );
        tab_proc[pid_cour-1].esi       = ( tab_proc[pid_cour-1].kernel_stack[nb_elem-13] );
        tab_proc[pid_cour-1].ebp       = ( tab_proc[pid_cour-1].kernel_stack[nb_elem-12] );
        tab_proc[pid_cour-1].edx       = ( tab_proc[pid_cour-1].kernel_stack[nb_elem-11] );
        tab_proc[pid_cour-1].ecx       = ( tab_proc[pid_cour-1].kernel_stack[nb_elem-10] );
        tab_proc[pid_cour-1].ebx       = ( tab_proc[pid_cour-1].kernel_stack[nb_elem-9] );
        tab_proc[pid_cour-1].eax       = ( tab_proc[pid_cour-1].kernel_stack[nb_elem-7] );
        tab_proc[pid_cour-1].ex_ebp    = ( tab_proc[pid_cour-1].kernel_stack[nb_elem-7] );
        tab_proc[pid_cour-1].ad_iret   = ( tab_proc[pid_cour-1].kernel_stack[nb_elem-6] );
        tab_proc[pid_cour-1].ad_proc   = ( tab_proc[pid_cour-1].kernel_stack[nb_elem-5] );
        tab_proc[pid_cour-1].cs        = ( tab_proc[pid_cour-1].kernel_stack[nb_elem-4] );
        tab_proc[pid_cour-1].eflags    = ( tab_proc[pid_cour-1].kernel_stack[nb_elem-3] );
        tab_proc[pid_cour-1].esp       = ( tab_proc[pid_cour-1].kernel_stack[nb_elem-2] );
        tab_proc[pid_cour-1].ss        = ( tab_proc[pid_cour-1].kernel_stack[nb_elem-1] );
        
        signalid=list_get_last(tab_proc[pid_cour-1].sigs);
        trt_sig=tab_proc[pid_cour-1].sigmap[signalid-1];

        tab_proc[pid_cour-1].kernel_stack[nb_elem-5] = (unsigned long) trt_sig;
        printf(" esp_user %x \n",(unsigned int)tab_proc[pid_cour-1].esp_user);
        tab_proc[pid_cour-1].esp_user -= 4;
        printf(" esp_user %x \n",(unsigned int)tab_proc[pid_cour-1].esp_user);
        *((unsigned long*)tab_proc[pid_cour-1].esp_user) = (unsigned long) adSigReturn;

        tab_proc[pid_cour-1].kernel_stack[nb_elem-2] = tab_proc[pid_cour-1].esp_user;
        tab_proc[pid_cour-1].sigmap[signalid-1]=(void*)ad_sig_default;
    }
    return;
}

void sig_return(void){
    size_t nb_elem= STACK_SIZE / sizeof(unsigned long);
    tab_proc[pid_cour-1].cur_sig=0;
        ( tab_proc[pid_cour-1].kernel_stack[nb_elem-14] )   = tab_proc[pid_cour-1].edi;
        ( tab_proc[pid_cour-1].kernel_stack[nb_elem-13] )   = tab_proc[pid_cour-1].esi;
        ( tab_proc[pid_cour-1].kernel_stack[nb_elem-12] )   = tab_proc[pid_cour-1].ebp;
        ( tab_proc[pid_cour-1].kernel_stack[nb_elem-11] )   = tab_proc[pid_cour-1].edx;
        ( tab_proc[pid_cour-1].kernel_stack[nb_elem-10] )   = tab_proc[pid_cour-1].ecx;
        ( tab_proc[pid_cour-1].kernel_stack[nb_elem-9] )    = tab_proc[pid_cour-1].ebx;
        ( tab_proc[pid_cour-1].kernel_stack[nb_elem-7] )    = tab_proc[pid_cour-1].eax;
        ( tab_proc[pid_cour-1].kernel_stack[nb_elem-7] )    = tab_proc[pid_cour-1].ex_ebp;
        ( tab_proc[pid_cour-1].kernel_stack[nb_elem-6] )    = tab_proc[pid_cour-1].ad_iret;
        ( tab_proc[pid_cour-1].kernel_stack[nb_elem-5] )    = tab_proc[pid_cour-1].ad_proc;
        ( tab_proc[pid_cour-1].kernel_stack[nb_elem-4] )    = tab_proc[pid_cour-1].cs;
        ( tab_proc[pid_cour-1].kernel_stack[nb_elem-3] )    = tab_proc[pid_cour-1].eflags;
        ( tab_proc[pid_cour-1].kernel_stack[nb_elem-2] )    = tab_proc[pid_cour-1].esp;
        ( tab_proc[pid_cour-1].kernel_stack[nb_elem-1] )    = tab_proc[pid_cour-1].ss;
    scheduler();
}

char* ad_sig_default = (char*) 0;


//********************************\Max
