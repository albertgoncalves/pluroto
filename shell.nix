with import <nixpkgs> {};
mkShell.override { stdenv = llvmPackages_17.stdenv; } {
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
