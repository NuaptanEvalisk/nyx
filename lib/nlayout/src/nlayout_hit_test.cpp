//
// Created by felix on 2/16/26.
//

#include "nlayout_internals.hpp"
#include <cassert>

std::pair<str_length_t, str_length_t> nlayout_next_meaningful(
    const std::pair<str_length_t, str_length_t> &p0,
    const str_length_t text_len,
    const nlayout_instruction_t *ins, const str_length_t ins_count)
{
  // next ibox
  for (str_length_t j1 = p0.second + 1; j1 < ins_count && ins[j1].offset == p0.first; ++j1)
  {
    if (ins[j1].asset_type == nlayout_asset_inline_box)
    {
      return { p0.first, j1 };
    }
  }
  return { std::min(text_len - 1, p0.first + 1), 0 };
}

NLayout_Error_Info nlayout_compute_hits(nlayout_runtime_t runtime,
                                        const str_length_t text_len,
                                        const nlayout_instruction_t *ins,
                                        const str_length_t ins_count)
{
  if (runtime == nullptr)
  {
    return nlayout_err_null_check;
  }
  if (runtime->ht_lanes.empty())
  {
    return nlayout_err_invalid_lanes;
  }

  // beware: the 'j' we store is the 'j' in global instruction sequence.
  str_length_t i = 0, j = 0;
  for (auto &lane : runtime->ht_lanes)
  {
    if (lane.inline_map.empty())
    {
      // dbox
      lane.inline_map.push_back({ {i, j } , -1 });
      const auto [fst, snd] =
          nlayout_next_meaningful({i, j},
            text_len, ins, ins_count);
      i = fst, j = snd;
    }
    else
    {
      // paragraph
      for (auto &key : lane.inline_map | std::views::keys)
      {
        key = { i, j };
        const auto [fst, snd] =
            nlayout_next_meaningful({i, j},
              text_len, ins, ins_count);
        i = fst, j = snd;
      }
    }
  }
  return nlayout_okay;
}

NLayout_Error_Info nlayout_get_hit(nlayout_runtime_t runtime,
                                   float x, float y, nlayout_hit_t *hit)
{
  if (runtime == nullptr)
  {
    return nlayout_err_null_check;
  }
  if (runtime->ht_lanes.empty())
  {
    return nlayout_err_invalid_lanes;
  }

  if (y < runtime->ht_lanes[0].top)
  {
    return nlayout_err_hit_out_of_range_up;
  }
  if (y > runtime->ht_lanes[runtime->ht_lanes.size() - 1].bottom)
  {
    return nlayout_err_hit_out_of_range_down;
  }

  for (const auto &lane : runtime->ht_lanes)
  {
    if (y < lane.top || y > lane.bottom)
    {
      continue;
    }
    if (lane.inline_map[0].second == -1)
    {
      // hit dbox.
      *hit = lane.inline_map[0].first;
      return nlayout_okay;
    }

    for (const auto &[fst, snd] : lane.inline_map)
    {
      if (x < snd)
      {
        *hit = fst;
        return nlayout_okay;
      }
    }
    *hit = lane.inline_map[lane.inline_map.size() - 1].first;
    return nlayout_okay;
  }
  // impossible to be here.
  assert(false);
  return nlayout_err_impossible;
}