#ifndef LIBTCC_H
#define LIBTCC_H

#ifndef LIBTCCAPI
# define LIBTCCAPI
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct TCCState;

typedef struct TCCState TCCState;

/* create a new TCC compilation context */
using tcc_new = TCCState *(*)(void);

/* free a TCC compilation context */
using tcc_delete = void (*)(TCCState *s);

/* set CONFIG_TCCDIR at runtime */
LIBTCCAPI void tcc_set_lib_path(TCCState *s, const char *path);

/* set error/warning display callback */
using tcc_set_error_func = void(*)(TCCState *s, void *error_opaque,
    void (*error_func)(void *opaque, const char *msg));

/* set options as from command line (multiple supported) */
using tcc_set_options = int(*)(TCCState *s, const char *str);

/*****************************/
/* preprocessor */

/* add include path */
using tcc_add_include_path = int(*)(TCCState *s, const char *pathname);

/* add in system include path */
LIBTCCAPI int tcc_add_sysinclude_path(TCCState *s, const char *pathname);

/* define preprocessor symbol 'sym'. Can put optional value */
using tcc_define_symbol = void(*)(TCCState *s, const char *sym, const char *value);

/* undefine preprocess symbol 'sym' */
LIBTCCAPI void tcc_undefine_symbol(TCCState *s, const char *sym);

/*****************************/
/* compiling */

/* add a file (C file, dll, object, library, ld script). Return -1 if error. */
LIBTCCAPI int tcc_add_file(TCCState *s, const char *filename, int filetype);
#define TCC_FILETYPE_BINARY 1
#define TCC_FILETYPE_C      2
#define TCC_FILETYPE_ASM    3
#define TCC_FILETYPE_ASM_PP 4

/* compile a string containing a C source. Return -1 if error. */
using tcc_compile_string = int (*)(TCCState *s, const char *buf);

/*****************************/
/* linking commands */

/* set output type. MUST BE CALLED before any compilation */
using tcc_set_output_type = int (*)(TCCState *s, int output_type);


#define TCC_OUTPUT_MEMORY   1 /* output will be run in memory (default) */
#define TCC_OUTPUT_EXE      2 /* executable file */
#define TCC_OUTPUT_DLL      3 /* dynamic library */
#define TCC_OUTPUT_OBJ      4 /* object file */
#define TCC_OUTPUT_PREPROCESS 5 /* only preprocess (used internally) */

/* equivalent to -Lpath option */
using tcc_add_library_path = int(*)(TCCState *s, const char *pathname);

/* the library name is the same as the argument of the '-l' option */
using tcc_add_library = int (*)(TCCState *s, const char *libraryname);

/* add a symbol to the compiled program */
using tcc_add_symbol = int(*)(TCCState *s, const char *name, const void *val);

/* output an executable, library or object file. DO NOT call
   tcc_relocate() before. */
LIBTCCAPI int tcc_output_file(TCCState *s, const char *filename);

/* link and run main() function and return its value. DO NOT call
   tcc_relocate() before. */
LIBTCCAPI int tcc_run(TCCState *s, int argc, char **argv);

/* do all relocations (needed before using tcc_get_symbol()) */
using tcc_relocate = int(*)(TCCState *s1, void *ptr);

/* possible values for 'ptr':
   - TCC_RELOCATE_AUTO : Allocate and manage memory internally
   - NULL              : return required memory size for the step below
   - memory address    : copy code to memory passed by the caller
   returns -1 if error. */
#define TCC_RELOCATE_AUTO (void*)1

/* return symbol value or NULL if not found */
using tcc_get_symbol = void *(*)(TCCState *s, const char *name);

#ifdef __cplusplus
}
#endif

#endif
