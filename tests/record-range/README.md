# recordRange tests

Tests `recordRange` from `template_cpp/src/src/receiver.cpp`.

```bash
tests/record-range/run.sh
```

Exit code 0 = all passed. Clean with `rm tests/record-range/test_bin`.

## Fixed cases

State before is `contiguous = 40` and `ranges = {44→49, 51→53}` unless noted.

| Test | Packet | New seqs | contiguous | ranges after |
|---|---|---|---|---|
| all old | 35–38 | none | 40 | {44→49, 51→53} |
| fast path (ranges empty) | 41–48 | 41–48 | 48 | {} |
| gap opens (ranges empty) | 44–49 | 44–49 | 40 | {44→49} |
| empty map, gap (ranges empty) | 60–62 | 60–62 | 40 | {60→62} |
| overlap | 50–55 | 50, 54, 55 | 40 | {44→55} |
| gap filled, absorb | 41–43 | 41, 42, 43 | 49 | {51→53} |
| messy retransmission | 38–45 | 41, 42, 43 | 49 | {51→53} |
| extend a range (ranges {44→49, 60→62}) | 50–52 | 50, 51, 52 | 40 | {44→52, 60→62} |
| duplicate of a stored range | 51–53 | none | 40 | {44→49, 51→53} |

## Random test

2000 runs of 60 random packets (seqs 1–127, 1–8 per packet), each checked against a plain `std::set` of every delivered seq: new seqs, `contiguous`, and ranges sorted with a gap before each.
