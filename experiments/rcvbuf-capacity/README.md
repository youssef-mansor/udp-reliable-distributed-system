# Receive buffer capacity

Counts how many UDP datagrams fit in a socket's receive buffer for each payload size (1–1000 B, plus a few larger), to size the sender window.

```bash
python3 experiments/rcvbuf-capacity/sweep.py
```

```
payload     1-  325 B:  222 packets  (~959 B charged each)
```

Results are saved to `results/<hostname>.txt`. Clean with `rm -rf experiments/rcvbuf-capacity/results`.
