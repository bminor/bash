
#if @HAVE_VISIBILITY@ && BUILDING_LIBINTL
#define LIBINTL_DLL_EXPORTED __attribute__((__visibility__("default")))
#elif defined _MSC_VER && BUILDING_LIBINTL
#define LIBINTL_DLL_EXPORTED __declspec(dllexport)
#else
#define LIBINTL_DLL_EXPORTED
#endif
