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

#include "mem/cache/replacement_policies/nmru_rp.hh"

#include <cassert>
#include <memory>

#include "params/NMRURP.hh"
#include "base/random.hh"
#include "sim/cur_tick.hh"

namespace gem5
{

GEM5_DEPRECATED_NAMESPACE(ReplacementPolicy, replacement_policy);
namespace replacement_policy
{

NMRU::NMRU(const Params &p)
  : Base(p)
{
}

void
NMRU::invalidate(const std::shared_ptr<ReplacementData>& replacement_data)
{
    // Reset last touch timestamp
    std::static_pointer_cast<NMRUReplData>(
        replacement_data)->lastTouchTick = Tick(0);
}

void
NMRU::touch(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    // Update last touch timestamp
    std::static_pointer_cast<NMRUReplData>(
        replacement_data)->lastTouchTick = curTick();
}

void
NMRU::reset(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    // Set last touch timestamp
    std::static_pointer_cast<NMRUReplData>(
        replacement_data)->lastTouchTick = curTick();
}

ReplaceableEntry*
NMRU::getVictim(const ReplacementCandidates& candidates) const
{
    // There must be at least one replacement candidate
    assert(candidates.size() > 0);

    std::shared_ptr<NMRUReplData> mru_replacement_data = 
        std::static_pointer_cast<NMRUReplData>(candidates[0]->replacementData);

    // Get MRU lastTouchTick
    for (const auto& candidate : candidates) {
        std::shared_ptr<NMRUReplData> candidate_replacement_data =
            std::static_pointer_cast<NMRUReplData>(candidate->replacementData);

        // Stop searching entry if a cache line that doesn't warm up is found.
        if (candidate_replacement_data->lastTouchTick == 0) {
            mru_replacement_data =
                std::static_pointer_cast<NMRUReplData>(candidate->replacementData);
            break;
        } else if (candidate_replacement_data->lastTouchTick > 
                        mru_replacement_data->lastTouchTick) {

            mru_replacement_data =
                std::static_pointer_cast<NMRUReplData>(candidate->replacementData);
        }
    }
    //printf("mru lst tick: %lu\n", mru_replacement_data->lastTouchTick);
    // Choose one candidate at random and make sure its not MRU
    ReplaceableEntry* victim;
    std::shared_ptr<NMRUReplData> victim_replacement_data;
    do{
        victim = candidates[random_mt.random<unsigned>(0, candidates.size() - 1)];
        victim_replacement_data =
            std::static_pointer_cast<NMRUReplData>(victim->replacementData);

        if(victim_replacement_data->lastTouchTick == 0){
            break;
        }
            //printf("vic: %lu\tmru: %lu\n",
            //    victim_replacement_data->lastTouchTick,
            //    mru_replacement_data->lastTouchTick);
        
    }while(victim_replacement_data->lastTouchTick == mru_replacement_data->lastTouchTick);
    

    //while(victim_replacement_data->lastTouchTick == 0){
        //victim = candidates[random_mt.random<unsigned>(0, candidates.size() - 1)];
        //victim = NMRU::getVictim(candidates);
    //}

    return victim;
}

std::shared_ptr<ReplacementData>
NMRU::instantiateEntry()
{
    return std::shared_ptr<ReplacementData>(new NMRUReplData());
}

} // namespace replacement_policy
} // namespace gem5
