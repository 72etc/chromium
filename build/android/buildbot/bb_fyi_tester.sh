#!/bin/bash -ex
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Buildbot annotator script for a FYI waterfall tester.
# Downloads and extracts a build from the builder and runs tests.

# SHERIFF: there should be no need to disable this bot.
# The FYI waterfall does not close the tree.

BB_DIR="$(dirname $0)"
BB_SRC_ROOT="$(cd  "$BB_DIR/../../.."; pwd)"
. "$BB_DIR/buildbot_functions.sh"

bb_baseline_setup "$BB_SRC_ROOT" "$@"
bb_extract_build
bb_reboot_phones
bb_run_unit_tests
bb_run_instrumentation_tests
