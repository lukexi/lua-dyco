#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "compile.h"
#include "process.h"
#include "dynamic.h"

library* CreateLibrary(
    char* Name,
    char* Source)
{
    library* Library = calloc(1, sizeof(library));
    Library->Name           = strdup(Name);
    Library->Source         = strdup(Source);
    Library->SourceUpdated  = true;

    snprintf(Library->Path, sizeof(Library->Path),
        "/tmp/%s.so",
        Library->Name);

    RecompileLibrary(Library);
    return Library;
}

void FreeLibrary(library* Library) {
    if (!Library) return;
    if (Library->LibHandle) {
        printf("Freeing and closing %s\n", Library->Path);
        dlclose(Library->LibHandle);
    }
    free(Library->Name);
    free(Library->Source);
    free(Library);
}

void* GetLibrarySymbol(library* Library, char* SymbolName) {
    if (!Library) return NULL;
    if (!Library->LibHandle) return NULL;
    return dlsym(Library->LibHandle, SymbolName);
}

bool UpdateLibrarySource(library* Library, char* Source) {
    if (!Library) return false;
    free(Library->Source);
    Library->Source = strdup(Source);
    Library->SourceUpdated = true;
    return RecompileLibrary(Library);
}

bool ReloadLibrary(library* Library) {
    if (!Library) return false;
    if (!Library->LibraryNeedsReload) return false;

    Library->LibraryNeedsReload = false;

    // Close the old library
    if (Library->LibHandle) {
        printf("Closing %s\n", Library->Path);
        int Result = dlclose(Library->LibHandle);
        if (Result) printf("dlclose error: %i\n", Result);

        void* LibraryHandle = dlopen(Library->Path, RTLD_NOLOAD);
        if (LibraryHandle) {
            printf("Library %s was still open after close!\n", Library->Path);
            dlclose(LibraryHandle);
        }
        Library->LibHandle = NULL;
    }

    // Open the new library
    void* NewLibraryHandle = NULL;

    if (Library->LibraryCompiledSuccessfully) {
        printf("Opening %s....\n", Library->Path);
        // Use RTLD_NOW so that unresolvable symbols result in an error.
        NewLibraryHandle = dlopen(Library->Path, RTLD_NOW);
        if (!NewLibraryHandle) {
            char* LibraryError = dlerror();
            printf("Error loading library %s: %s\n", Library->Path, LibraryError);
            // FIXME: append this to the compilation log.
        }
    }

    Library->LibHandle = NewLibraryHandle;

    return true;
}

time_t GetFileModTime(char* FileName) {
    struct stat attr = { 0 };
    stat(FileName, &attr);
    return attr.st_mtime;
}

bool RecompileLibrary(library* Library) {
    if (!Library) return false;

    if (IsCFileName(Library->Source)) {
        // If the source is a C file, check if it's been updated.
        time_t NewModTime = GetFileModTime(Library->Source);
        if (NewModTime == Library->LastModTime) return false;

        Library->LastModTime = NewModTime;
    } else if (!Library->SourceUpdated) {
        return false;
    }
    Library->SourceUpdated = false;

    // Clear the compilation log
    memset(Library->CompilationLog, 0, sizeof(Library->CompilationLog));

    // Compile the source into a library
    int ExitCode = CompileSource(
        Library->Path, Library->Source,
        Library->CompilationLog, sizeof(Library->CompilationLog),
        &Library->CompilationLogLength);

    if (Library->CompilationLogLength) {
        printf("Compilation log: %s\n", Library->CompilationLog);
    }

    Library->LibraryNeedsReload = true;
    Library->LibraryCompiledSuccessfully = ExitCode == 0;

    if (ExitCode != 0) {
        printf("Compilation failed with exit code: %i\n", ExitCode);
        return true;
    }

    return true;
}


// Shortcut to grabbing a single function.
void* DynamicFunction(char* FunctionName, char* FunctionSource) {

    char FileNameSO[256];
    snprintf(FileNameSO, sizeof(FileNameSO), "%s.so", FunctionName);

    // Check if the library is already open, and close it if so
    void* LibraryHandle = dlopen(FileNameSO, RTLD_NOLOAD);
    if (LibraryHandle) {
        // Must call dlclose twice, since dlopen with RTLD_NOLOAD
        // still increments the reference count
        int Result = dlclose(LibraryHandle);
        if (Result) printf("dlclose error: %i\n", Result);
        Result = dlclose(LibraryHandle);
        if (Result) printf("dlclose error: %i\n", Result);
    }

    library* Library = CreateLibrary(FunctionName, FunctionSource);

    void* FunctionHandle = GetLibrarySymbol(Library, FunctionName);

    // Prevent FreeLibrary from freeing the lib.
    Library->LibHandle = NULL;
    FreeLibrary(Library);

    return FunctionHandle;
}
