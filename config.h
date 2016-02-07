/* This is a generated file. */
#ifndef CONFIG_H_
#define CONFIG_H_

/* Probe byte-order via defines (clang & gcc at least work) to avoid run-time
 * tests */
#ifdef __BYTE_ORDER__
# ifdef __ORDER_LITTLE_ENDIAN__
#  if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#   define HAVE_LITTLE_ENDIAN 1
#  endif
# endif
# ifdef __ORDER_BIG_ENDIAN__
#  if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#   define HAVE_BIG_ENDIAN 1
#  endif
# endif
#endif


#ifndef HAVE_LITTLE_ENDIAN
# ifdef HAVE_BIG_ENDIAN
#  define HAVE_LITTLE_ENDIAN 0
# else
/* tcc is mean and doesn't give us any hints. Thankfully, it only supports LE */
#  ifdef __TINYC__
#   define HAVE_LITTLE_ENDIAN 1
#   define HAVE_BIG_ENDIAN 0
#  else
#   warning "No endian detected, try expanding tests"
#   define HAVE_LITTLE_ENDIAN 0
#   define HAVE_BIG_ENDIAN 0
#  endif
# endif
#else
# ifndef HAVE_BIG_ENDIAN
#  define HAVE_BIG_ENDIAN 0
# else
#  error "Unexpected"
# endif
#endif
#define HAVE_ATTRIBUTE_COLD 1
#define HAVE_ATTRIBUTE_CONST 1
#define HAVE_ATTRIBUTE_NORETURN 1
#define HAVE_ATTRIBUTE_PRINTF 1
#define HAVE_ATTRIBUTE_PURE 1
#define HAVE_ATTRIBUTE_UNUSED 1
#define HAVE_ATTRIBUTE_USED 1
#define HAVE_BUILTIN_CHOOSE_EXPR 1
#define HAVE_BUILTIN_CONSTANT_P 1
#define HAVE_BUILTIN_EXPECT 1
#define HAVE_BUILTIN_TYPES_COMPATIBLE_P 1
#define HAVE_ERR_H 1
#define HAVE_ISBLANK 1
#define HAVE_MEMMEM 1
#define HAVE_MEMRCHR 1
#define HAVE_TYPEOF 1
#define HAVE_WARN_UNUSED_RESULT 1
/* NOTE: note-NON_ATTRIBUTE.c "compiler errors when unrecognized attributes are used, no effect on config.h accuracy" */
/* warn-if-compiles-NON_FUNCTION.c build failed (as expected) */
#endif /* CONFIG_H_ */
