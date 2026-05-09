#!/usr/bin/env python3

"""Real k3s functional checks for meta_service_leader_election."""

from __future__ import annotations

import argparse
import importlib.util
import os
import sys
import threading
import time
import traceback
import uuid
from contextlib import suppress
from datetime import datetime, timedelta, timezone
from pathlib import Path
from typing import Callable, Optional
from unittest.mock import patch

from kubernetes import client, config
from kubernetes.client.rest import ApiException
from kubernetes.config.config_exception import ConfigException


REPO_ROOT = Path(__file__).resolve().parents[4]
TARGET_MODULE_PATH = REPO_ROOT / "src/memcache/python/memcache_hybrid/meta_service_leader_election.py"
DEFAULT_K3S_KUBECONFIG = "/etc/rancher/k3s/k3s.yaml"
DEFAULT_POD_IMAGE = "registry.k8s.io/pause:3.9"
DEFAULT_LEASE_DURATION_SECONDS = 10
DEFAULT_RETRY_PERIOD_SECONDS = 1
POLL_INTERVAL_SECONDS = 0.5
RESOURCE_WAIT_SECONDS = 30
CLEANUP_WAIT_SECONDS = 60
THREAD_WAIT_SECONDS = 12


class _NoOpMemcacheLib:
    def mmc_logger(self, level, message):
        return 0


def _print_pass(name: str) -> None:
    print(f"[PASS] {name}", flush=True)


def _print_fail(name: str, exc: BaseException) -> None:
    print(f"[FAIL] {name}: {exc}", file=sys.stderr, flush=True)
    traceback.print_exc()


def _load_target_module():
    module_name = "meta_service_leader_election_k3s_check"

    def _exec_module(with_fallback_logger: bool = False):
        spec = importlib.util.spec_from_file_location(module_name, TARGET_MODULE_PATH)
        if spec is None or spec.loader is None:
            raise RuntimeError(f"Failed to load module spec from {TARGET_MODULE_PATH}")

        module = importlib.util.module_from_spec(spec)
        if with_fallback_logger:
            with patch("ctypes.cdll.LoadLibrary", return_value=_NoOpMemcacheLib()):
                spec.loader.exec_module(module)
        else:
            spec.loader.exec_module(module)
        return module

    try:
        return _exec_module(with_fallback_logger=False)
    except OSError as exc:
        if "libmf_memcache.so" not in str(exc):
            raise
        print("[WARN] libmf_memcache.so is unavailable; reloading with no-op logger fallback", flush=True)
        return _exec_module(with_fallback_logger=True)


def _prepare_kube_environment() -> None:
    if not os.environ.get("KUBECONFIG") and Path(DEFAULT_K3S_KUBECONFIG).exists():
        os.environ["KUBECONFIG"] = DEFAULT_K3S_KUBECONFIG

    try:
        config.load_incluster_config()
        return
    except ConfigException:
        pass

    try:
        config.load_kube_config()
        return
    except ConfigException as exc:
        raise RuntimeError("Failed to load Kubernetes config from kubeconfig or in-cluster config") from exc


def _utc_now() -> datetime:
    return datetime.now(timezone.utc)


def _to_utc_datetime(module, value):
    return module._to_utc_datetime(value)


def _wait_until(description: str, predicate: Callable[[], object], timeout_seconds: int = RESOURCE_WAIT_SECONDS):
    deadline = time.time() + timeout_seconds
    last_error: Optional[BaseException] = None
    while time.time() < deadline:
        try:
            result = predicate()
            if result:
                return result
        except ApiException as exc:
            last_error = exc
            if exc.status != 404:
                raise
        time.sleep(POLL_INTERVAL_SECONDS)
    if last_error is not None:
        raise TimeoutError(f"Timed out waiting for {description}: {last_error}")
    raise TimeoutError(f"Timed out waiting for {description}")


def _wait_for_deleted(description: str, predicate: Callable[[], object], timeout_seconds: int = RESOURCE_WAIT_SECONDS):
    deadline = time.time() + timeout_seconds
    while time.time() < deadline:
        try:
            predicate()
        except ApiException as exc:
            if exc.status == 404:
                return
            raise
        time.sleep(POLL_INTERVAL_SECONDS)
    raise TimeoutError(f"Timed out waiting for deletion of {description}")


def _wait_for_namespace_read(namespace_api, namespace: str):
    return _wait_until(
        f"namespace {namespace}",
        lambda: namespace_api.read_namespace(namespace),
    )


def _wait_for_pod_read(core_api, namespace: str, pod_name: str):
    return _wait_until(
        f"pod {namespace}/{pod_name}",
        lambda: core_api.read_namespaced_pod(name=pod_name, namespace=namespace),
    )


def _wait_for_lease_read(coordination_api, namespace: str, lease_name: str):
    return _wait_until(
        f"lease {namespace}/{lease_name}",
        lambda: coordination_api.read_namespaced_lease(name=lease_name, namespace=namespace),
    )


def _set_lease_state(coordination_api, namespace: str, lease_name: str, holder_identity, renew_time, acquire_time, lease_duration_seconds):
    lease = coordination_api.read_namespaced_lease(name=lease_name, namespace=namespace)
    lease.spec.holder_identity = holder_identity
    lease.spec.renew_time = renew_time
    lease.spec.acquire_time = acquire_time
    lease.spec.lease_duration_seconds = lease_duration_seconds
    coordination_api.replace_namespaced_lease(name=lease_name, namespace=namespace, body=lease)
    return _wait_for_lease_read(coordination_api, namespace, lease_name)


def _assert_label_role(core_api, namespace: str, pod_name: str, expected_role: str) -> None:
    pod = core_api.read_namespaced_pod(name=pod_name, namespace=namespace)
    labels = pod.metadata.labels or {}
    assert labels.get("role") == expected_role, f"expected pod role={expected_role}, got {labels}"


def _assert_datetime_greater(new_value, old_value, message: str) -> None:
    assert new_value is not None, message
    assert old_value is not None, message
    assert new_value > old_value, message


def _make_election(module, namespace: str, lease_name: str, pod_name: str, retry_period: int = DEFAULT_RETRY_PERIOD_SECONDS):
    return module.MetaServiceLeaderElection(
        lease_name=lease_name,
        namespace=namespace,
        pod_name=pod_name,
        retry_period=retry_period,
    )


def _test_to_utc_datetime(module):
    z_value = module._to_utc_datetime("2026-04-28T12:34:56Z")
    assert z_value.tzinfo is not None
    assert z_value.utcoffset() == timedelta(0)

    naive_value = module._to_utc_datetime(datetime(2026, 4, 28, 12, 34, 56))
    assert naive_value.tzinfo is not None
    assert naive_value.utcoffset() == timedelta(0)

    aware_source = datetime(2026, 4, 28, 20, 34, 56, tzinfo=timezone(timedelta(hours=8)))
    aware_value = module._to_utc_datetime(aware_source)
    assert aware_value.tzinfo is not None
    assert aware_value.utcoffset() == timedelta(0)
    assert aware_value.hour == 12


def _test_constructor(module, namespace: str, lease_name: str, pod_name: str):
    election = _make_election(module, namespace, lease_name, pod_name)
    assert election.coordination_v1 is not None
    assert election.core_v1 is not None
    assert hasattr(election.coordination_v1, "read_namespaced_lease")
    assert hasattr(election.core_v1, "patch_namespaced_pod")


def _test_check_leader_status(module, election, coordination_api, namespace: str, lease_name: str):
    _set_lease_state(coordination_api, namespace, lease_name, None, None, None, DEFAULT_LEASE_DURATION_SECONDS)
    assert election.check_leader_status() == "None"

    current_time = _utc_now()
    _set_lease_state(coordination_api, namespace, lease_name, "other-pod", current_time, current_time, DEFAULT_LEASE_DURATION_SECONDS)
    assert election.check_leader_status() == "other-pod"

    expired_time = _utc_now() - timedelta(seconds=DEFAULT_LEASE_DURATION_SECONDS + 5)
    _set_lease_state(coordination_api, namespace, lease_name, "expired-pod", expired_time, expired_time, DEFAULT_LEASE_DURATION_SECONDS)
    assert election.check_leader_status() == "None"


def _test_update_lease(module, election, coordination_api, namespace: str, lease_name: str, pod_name: str):
    _set_lease_state(coordination_api, namespace, lease_name, None, None, None, DEFAULT_LEASE_DURATION_SECONDS)
    assert election.update_lease(False) is True
    lease = _wait_for_lease_read(coordination_api, namespace, lease_name)
    assert lease.spec.holder_identity == pod_name
    assert lease.spec.acquire_time is not None
    assert lease.spec.renew_time is not None

    before_renew = _to_utc_datetime(module, lease.spec.renew_time)
    time.sleep(1.1)
    assert election.update_lease(True) is True
    lease = _wait_for_lease_read(coordination_api, namespace, lease_name)
    after_renew = _to_utc_datetime(module, lease.spec.renew_time)
    _assert_datetime_greater(after_renew, before_renew, "renew_time did not advance after renewal")

    other_renew = _utc_now()
    _set_lease_state(coordination_api, namespace, lease_name, "other-pod", other_renew, other_renew, DEFAULT_LEASE_DURATION_SECONDS)
    assert election.update_lease(False) is False
    assert election._retry_update_lease(False) == 0

    expired_renew = _utc_now() - timedelta(seconds=DEFAULT_LEASE_DURATION_SECONDS + 5)
    _set_lease_state(coordination_api, namespace, lease_name, "other-pod", expired_renew, expired_renew, DEFAULT_LEASE_DURATION_SECONDS)
    assert election.update_lease(False) is True


def _test_retry_update_lease(module, election, coordination_api, namespace: str, lease_name: str, pod_name: str):
    _set_lease_state(coordination_api, namespace, lease_name, None, None, None, DEFAULT_LEASE_DURATION_SECONDS)
    assert election._retry_update_lease(False) == 1

    other_renew = _utc_now()
    _set_lease_state(coordination_api, namespace, lease_name, "other-pod", other_renew, other_renew, DEFAULT_LEASE_DURATION_SECONDS)
    assert election._retry_update_lease(False) == 0


def _test_pod_label_updates(module, election, core_api, namespace: str, pod_name: str):
    election.update_pod_to_master()
    _assert_label_role(core_api, namespace, pod_name, "master")

    election.update_pod_to_backup()
    _assert_label_role(core_api, namespace, pod_name, "backup")

    election._update_pod_label({"role": "master"})
    _assert_label_role(core_api, namespace, pod_name, "master")


def _test_check_and_update_leadership(module, election, coordination_api, core_api, namespace: str, lease_name: str, pod_name: str):
    _set_lease_state(coordination_api, namespace, lease_name, None, None, None, DEFAULT_LEASE_DURATION_SECONDS)
    election.is_leader = False
    election._check_and_update_leadership()
    assert election.is_leader is True
    _assert_label_role(core_api, namespace, pod_name, "master")

    other_renew = _utc_now()
    _set_lease_state(coordination_api, namespace, lease_name, "other-pod", other_renew, other_renew, DEFAULT_LEASE_DURATION_SECONDS)
    election.is_leader = True
    election._check_and_update_leadership()
    assert election.is_leader is False
    _assert_label_role(core_api, namespace, pod_name, "backup")


def _test_renew_loop(module, election, coordination_api, namespace: str, lease_name: str, pod_name: str):
    _set_lease_state(coordination_api, namespace, lease_name, pod_name, _utc_now(), _utc_now(), DEFAULT_LEASE_DURATION_SECONDS)
    initial_lease = _wait_for_lease_read(coordination_api, namespace, lease_name)
    initial_renew = _to_utc_datetime(module, initial_lease.spec.renew_time)
    election.is_leader = True
    election.stop_event.clear()
    election.retry_period = 1

    thread = threading.Thread(target=election._renew_lease, daemon=True)
    thread.start()
    try:
        def _read_renewed_lease():
            current_lease = _wait_for_lease_read(coordination_api, namespace, lease_name)
            if _to_utc_datetime(module, current_lease.spec.renew_time) > initial_renew:
                return current_lease
            return None

        lease = _wait_until(
            f"renewal of lease {namespace}/{lease_name}",
            _read_renewed_lease,
        )
        after_renew = _to_utc_datetime(module, lease.spec.renew_time)
        _assert_datetime_greater(after_renew, initial_renew, "renew_time did not advance while the renew loop was running")
    finally:
        election.stop_election()
        thread.join(THREAD_WAIT_SECONDS)
        assert not thread.is_alive(), "renew loop did not stop in time"

    lease_after_stop = _wait_for_lease_read(coordination_api, namespace, lease_name)
    after_stop = _to_utc_datetime(module, lease_after_stop.spec.renew_time)
    assert after_stop is not None


def _cleanup(
    core_api,
    coordination_api,
    namespace: str,
    lease_name: str,
    pod_name: str,
    delete_namespace: bool,
    keep_resources: bool,
    lease_created: bool,
    pod_created: bool,
) -> None:
    if keep_resources:
        return

    if lease_created:
        with suppress(ApiException):
            coordination_api.delete_namespaced_lease(name=lease_name, namespace=namespace)
        _wait_for_deleted(
            f"lease {namespace}/{lease_name}",
            lambda: coordination_api.read_namespaced_lease(name=lease_name, namespace=namespace),
            timeout_seconds=CLEANUP_WAIT_SECONDS,
        )
    if pod_created:
        with suppress(ApiException):
            core_api.delete_namespaced_pod(name=pod_name, namespace=namespace)
        _wait_for_deleted(
            f"pod {namespace}/{pod_name}",
            lambda: core_api.read_namespaced_pod(name=pod_name, namespace=namespace),
            timeout_seconds=CLEANUP_WAIT_SECONDS,
        )

    if delete_namespace:
        with suppress(ApiException):
            core_api.delete_namespace(name=namespace)


def _parse_args():
    parser = argparse.ArgumentParser(description="Real k3s functional checks for meta_service_leader_election")
    parser.add_argument("--namespace", default=None, help="Existing namespace to use; otherwise create a unique test namespace")
    parser.add_argument("--lease-name", default=None, help="Lease name to use")
    parser.add_argument("--pod-name", default=None, help="Pod name to use")
    parser.add_argument("--pod-image", default=DEFAULT_POD_IMAGE, help="Pod image used for object creation")
    parser.add_argument("--keep-resources", action="store_true", help="Keep created Kubernetes resources after the script exits")
    return parser.parse_args()


def main() -> int:
    args = _parse_args()
    _prepare_kube_environment()
    module = _load_target_module()

    core_api = client.CoreV1Api()
    coordination_api = client.CoordinationV1Api()

    resource_suffix = uuid.uuid4().hex[:8]
    namespace = args.namespace or f"mmc-leader-election-test-{resource_suffix}"
    lease_name = args.lease_name or f"mmc-leader-election-test-{resource_suffix}-lease"
    pod_name = args.pod_name or f"mmc-leader-election-test-{resource_suffix}-pod"
    create_namespace = args.namespace is None

    print(f"[INFO] using namespace={namespace}, lease={lease_name}, pod={pod_name}", flush=True)

    namespace_created = False
    lease_created = False
    pod_created = False
    try:
        if create_namespace:
            namespace_body = client.V1Namespace(metadata=client.V1ObjectMeta(name=namespace))
            core_api.create_namespace(body=namespace_body)
            namespace_created = True
            _wait_for_namespace_read(core_api, namespace)
        else:
            _wait_for_namespace_read(core_api, namespace)

        lease = client.V1Lease(
            metadata=client.V1ObjectMeta(name=lease_name),
            spec=client.V1LeaseSpec(
                holder_identity=None,
                lease_duration_seconds=DEFAULT_LEASE_DURATION_SECONDS,
                acquire_time=None,
                renew_time=None,
            ),
        )
        coordination_api.create_namespaced_lease(namespace=namespace, body=lease)
        lease_created = True
        _wait_for_lease_read(coordination_api, namespace, lease_name)

        pod = client.V1Pod(
            metadata=client.V1ObjectMeta(
                name=pod_name,
                labels={"app": "mmc-leader-election-test", "role": "backup"},
            ),
            spec=client.V1PodSpec(
                restart_policy="Never",
                containers=[
                    client.V1Container(
                        name="leader-election-check",
                        image=args.pod_image,
                    )
                ],
            ),
        )
        core_api.create_namespaced_pod(namespace=namespace, body=pod)
        pod_created = True
        _wait_for_pod_read(core_api, namespace, pod_name)

        election = _make_election(module, namespace, lease_name, pod_name)

        _test_to_utc_datetime(module)
        _print_pass("_to_utc_datetime parses Z, naive, aware datetimes")

        _test_constructor(module, namespace, lease_name, pod_name)
        _print_pass("constructor loads config and creates API clients")

        _test_check_leader_status(module, election, coordination_api, namespace, lease_name)
        _print_pass("check_leader_status handles empty, active, and expired leases")

        _test_update_lease(module, election, coordination_api, namespace, lease_name, pod_name)
        _print_pass("update_lease acquires, renews, and respects other holders")

        _test_retry_update_lease(module, election, coordination_api, namespace, lease_name, pod_name)
        _print_pass("_retry_update_lease returns success and no-op statuses")

        _test_pod_label_updates(module, election, core_api, namespace, pod_name)
        _print_pass("pod label updates switch between master and backup")

        _test_check_and_update_leadership(module, election, coordination_api, core_api, namespace, lease_name, pod_name)
        _print_pass("_check_and_update_leadership elects leader and downgrades backup")

        _test_renew_loop(module, election, coordination_api, namespace, lease_name, pod_name)
        _print_pass("renew loop starts, renews, and stops cleanly")

        print("[PASS] all k3s leader election checks passed", flush=True)
        return 0
    except Exception as exc:
        _print_fail("k3s leader election checks", exc)
        return 1
    finally:
        try:
            _cleanup(
                core_api,
                coordination_api,
                namespace,
                lease_name,
                pod_name,
                delete_namespace=namespace_created,
                keep_resources=args.keep_resources,
                lease_created=lease_created,
                pod_created=pod_created,
            )
        except Exception as cleanup_exc:
            print(f"[WARN] cleanup failed: {cleanup_exc}", file=sys.stderr, flush=True)


if __name__ == "__main__":
    sys.exit(main())
