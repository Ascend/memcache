# HA Python Test Scripts

This directory contains Python tests and runnable scripts for MemCache meta-service high-availability behavior.

## `test-mmc-meta-leader-election-k3s.py`

`test-mmc-meta-leader-election-k3s.py` is a live k3s/Kubernetes functional test for:

- `src/memcache/python/memcache_hybrid/meta_service_leader_election.py`

It uses the real Kubernetes API server and does not mock `kubernetes.client`. The script creates Kubernetes resources, exercises the leader-election helper, verifies results with assertions, and exits with a non-zero status on failure.

### What it checks

The script validates these paths:

- `_to_utc_datetime`
- `MetaServiceLeaderElection` construction and Kubernetes config loading
- `check_leader_status`
- `update_lease`
- `_retry_update_lease`
- `update_pod_to_master`
- `update_pod_to_backup`
- `_update_pod_label`
- `_check_and_update_leadership`
- `_renew_lease` background renewal loop

### Requirements

- A reachable k3s/Kubernetes cluster.
- Python Kubernetes client installed in the runtime environment.
- Permission to create/read/update/delete namespaces, Leases, and Pods.
- A valid Kubernetes config. If `KUBECONFIG` is not set and `/etc/rancher/k3s/k3s.yaml` exists, the script sets `KUBECONFIG` to that path automatically.

The script loads `meta_service_leader_election.py` directly from the source tree. If only `libmf_memcache.so` is unavailable, it falls back to a no-op logger so the real Kubernetes logic can still be tested.

### Basic usage

Run with an automatically created temporary namespace:

```bash
python3 test/python/memcache/ha/test-mmc-meta-leader-election-k3s.py
```

Run in an existing namespace with explicit resource names:

```bash
python3 test/python/memcache/ha/test-mmc-meta-leader-election-k3s.py \
  --namespace default \
  --lease-name mmc-leader-election-verify \
  --pod-name mmc-leader-election-verify
```

Keep created resources for debugging:

```bash
python3 test/python/memcache/ha/test-mmc-meta-leader-election-k3s.py --keep-resources
```

### Arguments

- `--namespace`: Existing namespace to use. If omitted, the script creates a unique temporary namespace.
- `--lease-name`: Lease name to use. If omitted, the script generates a unique test Lease name.
- `--pod-name`: Pod name to use. If omitted, the script generates a unique test Pod name.
- `--pod-image`: Image used when creating the test Pod. The default is `registry.k8s.io/pause:3.9`.
- `--keep-resources`: Do not delete resources created by the script.

### Resource cleanup

By default, the script deletes only resources it successfully created:

- If it creates a temporary namespace, it requests namespace deletion during cleanup.
- If `--namespace` points to an existing namespace, it does not delete that namespace.
- It tracks whether the Lease and Pod were created by the script before deleting them, to avoid deleting pre-existing resources with the same names.

### Success output

A successful run prints one `[PASS]` line for each scenario and ends with:

```text
[PASS] all k3s leader election checks passed
```
