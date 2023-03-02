/**
 * Copyright (c) 2018-2020 Inria
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "mem/cache/replacement_policies/nru_rp.hh"

#include <cassert>
#include <memory>

//#include "base/logging.hh" // For fatal_if
#include "base/random.hh"
#include "params/NRURP.hh"

namespace gem5
{

GEM5_DEPRECATED_NAMESPACE(ReplacementPolicy, replacement_policy);
namespace replacement_policy
{

NRU::NRU(const Params &p)
  : Base(p)
{
}

void
NRU::invalidate(const std::shared_ptr<ReplacementData>& replacement_data)
{
    std::shared_ptr<NRUReplData> casted_replacement_data =
        std::static_pointer_cast<NRUReplData>(replacement_data);

    // Invalidate entry
    casted_replacement_data->valid = false;
}

void
NRU::touch(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    std::shared_ptr<NRUReplData> casted_replacement_data =
        std::static_pointer_cast<NRUReplData>(replacement_data);

    // Update NRU bit if not 0 yet
    if (casted_replacement_data->nru_bit.isSaturated() == false) {
        casted_replacement_data->nru_bit--;
    }
}

void
NRU::reset(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    std::shared_ptr<NRUReplData> casted_replacement_data =
        std::static_pointer_cast<NRUReplData>(replacement_data);

    // Reset NRU bit
    casted_replacement_data->nru_bit.reset();
    
    // Mark entry as ready to be used
    casted_replacement_data->valid = true;
}

ReplaceableEntry*
NRU::getVictim(const ReplacementCandidates& candidates) const
{
    // There must be at least one replacement candidate
    assert(candidates.size() > 0);

    // Use first candidate as dummy victim
    ReplaceableEntry* victim = nullptr;

    // Visit all candidates to search for an invalid entry. If one is found,
    // its eviction is prioritized
    for (const auto& candidate : candidates) {
        if (!std::static_pointer_cast<NRUReplData>(
                    candidate->replacementData)->valid) {
            victim = candidate;
            return victim;
        }
    }

    // Visit all candidates to find victim
    for (const auto& candidate : candidates) {
        std::shared_ptr<NRUReplData> candidate_repl_data =
            std::static_pointer_cast<NRUReplData>(
                candidate->replacementData);

        // Find first canidate that is marked as !near future from the... LEFT!
        //  ~~~ Everything ya own in the box to the left ~~~
        if (candidate_repl_data->nru_bit.isSaturated()) {
            victim = candidate;
            break;
        }
    }
    
    // No suitable victims where found - reset all nru_bits to be !near future
    if(victim == nullptr){
        for (const auto& candidate : candidates) {
            std::shared_ptr<NRUReplData> candidate_repl_data =
                std::static_pointer_cast<NRUReplData>(
                    candidate->replacementData);
            candidate_repl_data->nru_bit.reset();
        }

        // All canidates nru_bit set to not be used in near future
        // All are valid canidates so return left most
        victim = candidates[0];
    }
    return victim;
}

std::shared_ptr<ReplacementData>
NRU::instantiateEntry()
{
    return std::shared_ptr<ReplacementData>(new NRUReplData());
}

} // namespace replacement_policy
} // namespace gem5
