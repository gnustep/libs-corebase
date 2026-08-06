#!/usr/bin/env bash

set -u

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
REPO_ROOT=$(CDPATH= cd -- "$SCRIPT_DIR/../.." && pwd)
LOG_FILE="$REPO_ROOT/Tests/tests.log"

die() {
  echo "error: $*" >&2
  exit 2
}

source_gnustep() {
  set +u
  if [ -n "${GNUSTEP_SH:-}" ]; then
    . "$GNUSTEP_SH"
  elif [ -n "${INSTALL_PATH:-}" ] && [ -f "$INSTALL_PATH/share/GNUstep/Makefiles/GNUstep.sh" ]; then
    . "$INSTALL_PATH/share/GNUstep/Makefiles/GNUstep.sh"
  elif [ -f /opt/gnustep/share/GNUstep/Makefiles/GNUstep.sh ]; then
    . /opt/gnustep/share/GNUstep/Makefiles/GNUstep.sh
  fi
  set -u

  if [ -z "${GNUSTEP_MAKEFILES:-}" ]; then
    GNUSTEP_MAKEFILES=$(gnustep-config --variable=GNUSTEP_MAKEFILES 2>/dev/null || true)
    export GNUSTEP_MAKEFILES
  fi

  [ -n "${GNUSTEP_MAKEFILES:-}" ] || die "GNUSTEP_MAKEFILES is not set; source GNUstep.sh first"
}

print_failure_summary() {
  [ -f "$LOG_FILE" ] || return 0

  if ! grep -Eq '^(Failed test:|Failed file:)' "$LOG_FILE"; then
    return 0
  fi

  printf "\nFailure summary:\n"
  awk '
    /^Failed test:/ || /^Failed file:/ {
      print
      show = 1
      next
    }
    show && (/^expected / || /^but got / || /^Expected:/ || /^Actual:/) {
      print
      next
    }
    show && (/^(Passed test:|Dashed hope:|Skipped file:|Testing |--- Running)/) {
      show = 0
    }
  ' "$LOG_FILE"
  printf "\nFull log: %s\n" "$LOG_FILE"
}

print_full_log_group() {
  [ -f "$LOG_FILE" ] || return 0

  printf "\n::group::Full Linux test log\n"
  cat "$LOG_FILE"
  printf "::endgroup::\n"
}

main() {
  source_gnustep

  # Run the tests and extract summary
  cd "$REPO_ROOT/Tests" || exit 2
  if ! make check; then
    print_failure_summary
    print_full_log_group
    exit 1
  fi
}

main "$@"
