"""Use the upstream host checks with time for async UI delivery after rendering.
The run() helper gains a wait and callback counters become atomic.
Assertions and production code are unchanged.
"""
def source(build,dest):
    text=(build.MPC/'tools/host_test.c').read_text()
    marker='    for (int k = 0; k < blocks; k++) a->pr(a, ip, o, 128);\n'
    assert text.count(marker)==1,'upstream test helper changed; review async adaptation'
    text=text.replace(marker,marker+'#if ASYNC_HOST_NOTIFY\n    usleep(20000);\n#endif\n')
    text=text.replace('#include <stdio.h>','#include <stdio.h>\n#include <stdatomic.h>')
    text=text.replace('static int automated[NPARAMS > 0 ? NPARAMS : 1], fails, updates;', 'static atomic_int automated[NPARAMS > 0 ? NPARAMS : 1], updates; static int fails;')
    text=text.replace('memset(automated, 0, sizeof automated);','for(int q=0;q<NPARAMS;++q)atomic_store(&automated[q],0);')
    path=dest/'framework_host_test.c';path.write_text(text);return path
