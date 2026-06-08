#!/usr/bin/env bash

DefaultCMakePreset() {
   case "$(uname -s)" in
      MINGW* | MSYS* | CYGWIN*)
         printf "%s\n" "msvc-debug"
         ;;
      *)
         printf "%s\n" "debug"
         ;;
   esac
}

CommandExists() {
   command -v "$1" >/dev/null 2>&1
}

IsWindows() {
   case "$(uname -s)" in
      MINGW* | MSYS* | CYGWIN*)
         return 0
         ;;
      *)
         return 1
         ;;
   esac
}

IsLinux() {
   [[ "$(uname -s)" == "Linux" ]]
}
