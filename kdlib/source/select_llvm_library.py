import os, sys, re

llvm_libs = ['Analysis', 'AsmParser', 'BinaryFormat', 'Bitcode', 'Bitstream',
             'CodeGenTypes', 'DebugInfo', 'Demangle', 'Frontend', 'IR', 'IRReader',
             'MC', 'Object', 'Option', 'ProfileData', 'Remarks',
             'Support', 'Target', 'TargetParser', 'TextAPI', 'Transforms', 'WindowsDriver']
clang_libs = ['Analysis', 'APINotes', 'AST', 'ASTMatchers', 'Basic',
              'Driver', 'Edit', 'Frontend', 'Lex', 'Parse', 'Sema',
              'Serialization', 'Support', 'Tooling']

def main():
    if(len(sys.argv) >= 2) and os.path.isdir(sys.argv[1]):
        llvm_root = sys.argv[1]

        print(f"Enabled LLVM libs: {llvm_libs}")
        llvm_lib_cmakelist = os.path.join(llvm_root, "llvm", "lib", "CMakeLists.txt")
        if os.path.isfile(llvm_lib_cmakelist):
            lines = []
            with open(llvm_lib_cmakelist, 'r', newline = '') as f_llvm_libs:
                lines = f_llvm_libs.readlines()

            ln = 0
            for l in lines:
                l_add_lib = re.findall(r"^add_subdirectory\((.*)\)$", l, flags = re.IGNORECASE)
                if (len(l_add_lib)) and (l_add_lib[0].lower() not in [x.lower() for x in llvm_libs]):
                    l_remove_lib = "# " + l
                    lines[ln] = l_remove_lib
                ln += 1

            with open(llvm_lib_cmakelist, 'w', newline = '') as f_llvm_libs:
                f_llvm_libs.writelines(lines)

        print(f"Enabled Clang libs: {clang_libs}")
        clang_lib_cmakelist = os.path.join(llvm_root, "clang", "lib", "CMakeLists.txt")
        if os.path.isfile(clang_lib_cmakelist):
            lines = []
            with open(clang_lib_cmakelist, 'r', newline = '') as f_clang_libs:
                lines = f_clang_libs.readlines()

            ln = 0
            for l in lines:
                l_add_lib = re.findall(r"^add_subdirectory\((.*)\)$", l, flags = re.IGNORECASE)
                if (len(l_add_lib)) and (l_add_lib[0].lower() not in [x.lower() for x in clang_libs]):
                    l_remove_lib = "# " + l
                    lines[ln] = l_remove_lib
                ln += 1

            with open(clang_lib_cmakelist, 'w', newline = '') as f_clang_libs:
                f_clang_libs.writelines(lines)
    else:
        print("Expect a valid llvm-project root")

if __name__ == '__main__':
    main()