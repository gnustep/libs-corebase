#!/bin/sh

set -u

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
REPO_ROOT=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)
LOG_FILE="$SCRIPT_DIR/tests.log"

die() {
  echo "error: $*" >&2
  exit 2
}

source_gnustep() {
  set +u
  if [ -n "${GNUSTEP_SH:-}" ]; then
    # shellcheck disable=SC1090
    . "$GNUSTEP_SH"
  elif [ -n "${INSTALL_PATH:-}" ] && [ -f "$INSTALL_PATH/share/GNUstep/Makefiles/GNUstep.sh" ]; then
    # shellcheck disable=SC1090
    . "$INSTALL_PATH/share/GNUstep/Makefiles/GNUstep.sh"
  elif [ -f /opt/gnustep/share/GNUstep/Makefiles/GNUstep.sh ]; then
    # shellcheck disable=SC1091
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

  cd "$REPO_ROOT" || exit 2
  make

  cd "$SCRIPT_DIR" || exit 2
  if ! make check; then
    print_failure_summary
    print_full_log_group
    exit 1
  fi
}

main "$@"
