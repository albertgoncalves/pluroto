with import <nixpkgs> {};
mkShell.override { stdenv = llvmPackages_18.stdenv; } {
    buildInputs = [
        gdb
        shellcheck
        valgrind
    ];
    shellHook = ''
        . .shellhook
    '';
    hardeningDisable = [ "all" ];
}
