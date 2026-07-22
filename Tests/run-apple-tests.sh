#!/bin/sh

set -u

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
REPO_ROOT=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)
DEFAULT_TEST_ROOT="$SCRIPT_DIR"
TMPDIR_ROOT="${TMPDIR:-/tmp}"
WORK_DIR=$(mktemp -d "$TMPDIR_ROOT/corebase-apple-tests.XXXXXX")
LOG_FILE="$SCRIPT_DIR/apple-tests.log"
SUM_FILE="$SCRIPT_DIR/apple-tests.sum"
SDKROOT=$(xcrun --sdk macosx --show-sdk-path)

cleanup() {
  rm -rf "$WORK_DIR"
}
trap cleanup EXIT INT TERM HUP

PASSED_TESTS=0
FAILED_TESTS=0
DASHED_HOPES=0
FAILED_FILES=0
SKIPPED_FILES=0

die() {
  echo "error: $*" >&2
  exit 2
}

contains_word() {
  needle=$1
  shift
  for word in "$@"; do
    if [ "$word" = "$needle" ]; then
      return 0
    fi
  done
  return 1
}

append_output() {
  printf "%s\n" "$1" >>"$LOG_FILE"
}

append_summary_matches() {
  printf "%s\n" "$1" | awk '
    /^Passed test:/ ||
    /^Failed test:/ ||
    /^Failed file:/ ||
    /^Dashed hope:/ ||
    /^Skipped file:/ { print }
  ' >>"$SUM_FILE"
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

run_test_file() {
  dir=$1
  testfile=$2
  testname=$(basename "$testfile")
  binary_name=$(basename "$testfile")
  binary_name=${binary_name%.*}
  output_file="$WORK_DIR/$binary_name.out"
  binary_file="$WORK_DIR/$binary_name.bin"

  APPLE_SKIP_TESTS=""
  if [ -f "$dir/TestInfo" ]; then
    # shellcheck disable=SC1090
    . "$dir/TestInfo" >/dev/null 2>&1 || die "failed to source $dir/TestInfo"
  fi

  if contains_word "$testname" $APPLE_SKIP_TESTS; then
    printf "Skipped file:    %s\n" "$testname" | tee -a "$SUM_FILE" >>"$LOG_FILE"
    SKIPPED_FILES=$((SKIPPED_FILES + 1))
    return 0
  fi

  printf "Testing %s...\n" "$testname" | tee -a "$LOG_FILE"

  compile_cmd="xcrun --sdk macosx clang -isysroot \"$SDKROOT\" -I \"$SCRIPT_DIR\" -framework Foundation -framework CoreFoundation -fno-objc-arc -o \"$binary_file\" \"$testfile\""
  if ! sh -c "$compile_cmd" >"$output_file" 2>&1; then
    {
      printf "Failed file:  %s failed to compile!\n" "$testname"
      cat "$output_file"
    } | tee -a "$LOG_FILE" >/dev/null
    printf "Failed file:  %s failed to compile!\n" "$testname" >>"$SUM_FILE"
    FAILED_FILES=$((FAILED_FILES + 1))
    return 0
  fi

  if ! python3 - "$binary_file" >"$output_file" 2>&1 <<'PY'
import subprocess
import sys

binary = sys.argv[1]
try:
    result = subprocess.run([binary], check=False, timeout=15)
    sys.exit(result.returncode)
except subprocess.TimeoutExpired:
    sys.exit(124)
PY
  then
    status=$?
  else
    status=0
  fi

  cat "$output_file" | tee -a "$LOG_FILE" >/dev/null
  append_summary_matches "$(cat "$output_file")"

  passed=$(grep -c '^Passed test:' "$output_file" || true)
  failed=$(grep -c '^Failed test:' "$output_file" || true)
  hoped=$(grep -c '^Dashed hope:' "$output_file" || true)

  PASSED_TESTS=$((PASSED_TESTS + passed))
  FAILED_TESTS=$((FAILED_TESTS + failed))
  DASHED_HOPES=$((DASHED_HOPES + hoped))

  if [ "$status" -eq 124 ]; then
    printf "Failed file:  %s timed out!\n" "$testname" | tee -a "$LOG_FILE" >>"$SUM_FILE"
    FAILED_FILES=$((FAILED_FILES + 1))
    return 0
  fi

  if [ "$status" -ne 0 ]; then
    printf "Failed file:  %s exited with status %s!\n" "$testname" "$status" | tee -a "$LOG_FILE" >>"$SUM_FILE"
    FAILED_FILES=$((FAILED_FILES + 1))
    return 0
  fi
}

discover_source_dirs() {
  test_root=$1
  find "$test_root" -type f \( -name '*.m' -o -name '*.c' \) -print |
    sed -e 's;/[^/]*$;;' |
    sort -u
}

discover_test_files() {
  dir=$1
  find "$dir" -mindepth 1 -maxdepth 1 -type f \( -name '*.m' -o -name '*.c' \) -print |
    sort
}

run_test_dir() {
  dir=$1
  [ -f "$dir/TestInfo" ] || return 0

  rel_dir=${dir#"$REPO_ROOT"/}
  printf -- "--- Running tests in %s ---\n" "$rel_dir" | tee -a "$LOG_FILE"

  found_any=no
  for testfile in $(discover_test_files "$dir"); do
    found_any=yes
    run_test_file "$dir" "$testfile"
  done

  if [ "$found_any" = no ]; then
    printf "No tests found in %s\n" "$rel_dir" | tee -a "$LOG_FILE"
  fi
}

main() {
  : >"$LOG_FILE"
  : >"$SUM_FILE"

  if [ "$#" -eq 0 ]; then
    set -- "$DEFAULT_TEST_ROOT"
  fi

  found_any=no
  for target in "$@"; do
    case $target in
      /*) resolved_target=$target ;;
      *) resolved_target=$REPO_ROOT/$target ;;
    esac

    if [ -f "$resolved_target" ]; then
      dir=$(dirname "$resolved_target")
      [ -f "$dir/TestInfo" ] || die "no TestInfo found for $resolved_target"
      found_any=yes
      run_test_file "$dir" "$resolved_target"
      continue
    fi

    [ -d "$resolved_target" ] || die "path not found: $target"

    while IFS= read -r dir; do
      [ -f "$dir/TestInfo" ] || continue
      found_any=yes
      run_test_dir "$dir"
    done <<EOF
$(discover_source_dirs "$resolved_target")
EOF
  done

  if [ "$found_any" = no ]; then
    die "no test subdirectories found"
  fi

  printf "\n%7d Passed tests\n" "$PASSED_TESTS"
  printf "%7d Failed tests\n" "$FAILED_TESTS"
  printf "%7d Dashed hopes\n" "$DASHED_HOPES"
  printf "%7d Failed files\n" "$FAILED_FILES"
  printf "%7d Skipped files\n" "$SKIPPED_FILES"

  if [ "$FAILED_TESTS" -ne 0 ] || [ "$FAILED_FILES" -ne 0 ]; then
    print_failure_summary
    exit 1
  fi
}

main "$@"
