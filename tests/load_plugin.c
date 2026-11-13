/* Run host tests against the actual shared object, rather than a statically linked wrapper. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
typedef struct Effect Effect;
typedef intptr_t (*Callback)(Effect*,int32_t,int32_t,intptr_t,void*,float);
typedef Effect* (*Entry)(Callback);
static void* library;
static Entry entry;
Effect* VSTPluginMain(Callback host) { return entry(host); }
extern int plugin_test_main(void);
int main(int argc, char** argv) {
    if(argc!=2) { fputs("expected plugin path\n",stderr); return 1; }
    library=dlopen(argv[1],RTLD_NOW|RTLD_LOCAL);
    if(!library) { fprintf(stderr,"dlopen: %s\n",dlerror()); return 1; }
    entry=(Entry)dlsym(library,"VSTPluginMain");
    if(!entry) { fputs("VSTPluginMain missing\n",stderr); return 1; }
    const int result=plugin_test_main();
    dlclose(library);
    return result;
}
