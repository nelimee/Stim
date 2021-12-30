/*
 * Copyright 2021 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _STIM_SIMULATORS_MEASUREMENTS_TO_DETECTION_EVENTS_H
#define _STIM_SIMULATORS_MEASUREMENTS_TO_DETECTION_EVENTS_H

#include <cassert>
#include <functional>
#include <iostream>
#include <new>
#include <random>
#include <sstream>

#include "stim/circuit/circuit.h"
#include "stim/io/measure_record.h"
#include "stim/simulators/vector_simulator.h"
#include "stim/stabilizers/tableau.h"
#include "stim/stabilizers/tableau_transposed_raii.h"

namespace stim {

/// Reads measurement data from a file, converts it to detection event data, and writes that out to another file.
///
/// Args:
///     measurements_in: The file to read measurement data from.
///     input_format: The format of the measurement data in the file.
///     optional_sweep_bits_in: An optional file containing sweep data for each shot in the measurements file.
///     sweep_bits_in_format: The format of the sweep data file. Ignored when optional_sweep_bits_in == nullptr.
///     results_out: The file to write detection event data to.
///     output_format: The format to use when writing the detection event data.
///     circuit: The circuit that the measurement data corresponds to, with DETECTOR and OBSERVABLE_INCLUDE annotations
///         indicating how to perform the conversion.
///     append_observables: Whether or not to include observable flip data in the detection event data.
///     skip_reference_sample: When set to True, the reference sample used by the conversion is initialized to
///         all-zeroes instead of being collected from the circuit. This should probably only be done if you know the
///         all-zero sample is a valid sample, or if you know that the measurements were generated by a frame simulator
///         that was also incorrectly assuming an all-zero reference sample.
void stream_measurements_to_detection_events(
    FILE *measurements_in,
    SampleFormat measurements_in_format,
    FILE *optional_sweep_bits_in,
    SampleFormat sweep_bits_in_format,
    FILE *results_out,
    SampleFormat results_out_format,
    const Circuit &circuit,
    bool append_observables,
    bool skip_reference_sample);
/// A variant of `stim::stream_measurements_to_detection_events` with derived values passed in, not recomputed.
void stream_measurements_to_detection_events_helper(
    FILE *measurements_in,
    SampleFormat measurements_in_format,
    FILE *optional_sweep_bits_in,
    SampleFormat sweep_bits_in_format,
    FILE *results_out,
    SampleFormat results_out_format,
    const Circuit &circuit,
    bool append_observables,
    simd_bits_range_ref reference_sample,
    size_t num_measurements,
    size_t num_observables,
    size_t num_detectors,
    size_t num_qubits,
    size_t num_sweep_bits);

/// Converts measurement data into detection event data based on a circuit.
///
/// Args:
///     measurements__minor_shot_index: Recorded measurement data.
///         Major axis: measurement bit index.
///         Minor axis: shot index.
///     sweep_bits__minor_shot_index: Per-shot configuration data controlling operations like `CNOT sweep[0] 1`.
///         Major axis: sweep bit index.
///         Minor axis: shot index.
///
///         To not specify sweep data, set the major axis length of sweep_bits__minor_shot_index to 0 (the minor axis
///         length must still match the number of shots). The major axis can also be partially truncated. Sweep bits
///         beyond that length default to False.
///     circuit: The circuit that the measurement data corresponds to, with DETECTOR and OBSERVABLE_INCLUDE annotations
///         indicating how to perform the conversion.
///     append_observables: Whether or not to include observable flip data in the detection event data.
///     skip_reference_sample: When set to True, the reference sample used by the conversion is initialized to
///         all-zeroes instead of being collected from the circuit. This should probably only be done if you know the
///         all-zero sample is a valid sample, or if you know that the measurements were generated by a frame simulator
///         that was also incorrectly assuming an all-zero reference sample.
///
/// Returns:
///     Detection event data. Major axis is detector index (+ observable index). Minor axis is shot index.
simd_bit_table measurements_to_detection_events(
    const simd_bit_table &measurements__minor_shot_index,
    const simd_bit_table &sweep_bits__minor_shot_index,
    const Circuit &circuit,
    bool append_observables,
    bool skip_reference_sample);
/// A variant of `stim::measurements_to_detection_events` with derived values passed in, not recomputed.
void measurements_to_detection_events_helper(
    const simd_bit_table &measurements__minor_shot_index,
    const simd_bit_table &sweep_bits__minor_shot_index,
    simd_bit_table &out_detection_results__minor_shot_index,
    const Circuit &noiseless_circuit,
    const simd_bits &reference_sample,
    bool append_observables,
    size_t num_measurements,
    size_t num_detectors,
    size_t num_observables,
    size_t num_qubits);

}  // namespace stim

#endif
