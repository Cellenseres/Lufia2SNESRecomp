#!/usr/bin/env python3
"""Create deterministic effective SNESRecomp cfg files from decomp metadata."""

from __future__ import annotations

import argparse
import json
import re
import sys
from dataclasses import dataclass
from pathlib import Path

try:
    import tomllib
except ModuleNotFoundError:  # pragma: no cover - Python 3.10 compatibility
    try:
        import tomli as tomllib
    except ModuleNotFoundError:  # pragma: no cover - dependency-free fallback
        tomllib = None


ADDRESS_RE = re.compile(r"^[0-9A-Fa-f]{2}:[0-9A-Fa-f]{4}$")
PC16_RE = re.compile(r"^[0-9A-Fa-f]{4}$")
SYMBOL_RE = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")
BANK_RE = re.compile(r"^\s*bank\s*=\s*([0-9A-Fa-f]{2})\s*(?:#.*)?$")
HLE_RE = re.compile(r"^\s*hle_func\s+(\S+)\s+(\S+)\s*(?:#.*)?$")
STATUSES = {"identified", "draft", "verified", "disabled"}


class ManifestError(ValueError):
    pass


@dataclass(frozen=True)
class Function:
    address: str
    name: str
    source: str
    entry_mx: str
    exit_mx: str
    status: str


@dataclass(frozen=True)
class Binding:
    address: str
    bridge: str
    allow_draft: bool


@dataclass(frozen=True)
class CfgFile:
    path: Path
    bank: str
    text: str


def _load_toml(path: Path) -> dict:
    try:
        if tomllib is not None:
            with path.open("rb") as stream:
                return tomllib.load(stream)
        return _load_basic_toml(path.read_text(encoding="utf-8"), path)
    except (OSError, ValueError) as exc:
        raise ManifestError(f"cannot parse {path}: {exc}") from exc


def _load_basic_toml(text: str, path: Path) -> dict:
    """Parse the intentionally small metadata schema on Python 3.10.

    Full TOML is used when tomllib/tomli is available. The fallback accepts
    top-level scalar keys plus arrays of tables, which is the complete schema
    used by functions.toml and decomp_bindings.toml.
    """
    document: dict = {}
    current = document
    for line_number, raw_line in enumerate(text.splitlines(), 1):
        line = raw_line.strip()
        if not line or line.startswith("#"):
            continue
        table = re.fullmatch(r"\[\[([A-Za-z_][A-Za-z0-9_]*)\]\]", line)
        if table:
            current = {}
            document.setdefault(table.group(1), []).append(current)
            continue
        assignment = re.fullmatch(r"([A-Za-z_][A-Za-z0-9_]*)\s*=\s*(.+)", line)
        if not assignment:
            raise ValueError(f"{path}:{line_number}: unsupported TOML syntax")
        key, raw_value = assignment.groups()
        if raw_value.startswith('"') and raw_value.endswith('"'):
            value = json.loads(raw_value)
        elif re.fullmatch(r"[+-]?[0-9]+", raw_value):
            value = int(raw_value, 10)
        elif raw_value in {"true", "false"}:
            value = raw_value == "true"
        else:
            raise ValueError(f"{path}:{line_number}: unsupported TOML value")
        if key in current:
            raise ValueError(f"{path}:{line_number}: duplicate key {key}")
        current[key] = value
    return document


def _format_address(value: object, context: str) -> str:
    if not isinstance(value, str) or not ADDRESS_RE.fullmatch(value):
        raise ManifestError(
            f"{context}: address must use BB:AAAA hexadecimal form, got {value!r}"
        )
    return value.upper()


def _symbol(value: object, context: str) -> str:
    if not isinstance(value, str) or not SYMBOL_RE.fullmatch(value):
        raise ManifestError(f"{context}: invalid C symbol {value!r}")
    return value


def _require_string(record: dict, key: str, context: str) -> str:
    value = record.get(key)
    if not isinstance(value, str) or not value:
        raise ManifestError(f"{context}: {key} must be a non-empty string")
    return value


def load_functions(path: Path) -> list[Function]:
    document = _load_toml(path)
    if document.get("format") != 1:
        raise ManifestError(f"{path}: expected format = 1")
    records = document.get("function", [])
    if not isinstance(records, list):
        raise ManifestError(f"{path}: function must be an array of tables")

    functions: list[Function] = []
    addresses: set[str] = set()
    names: set[str] = set()
    for index, record in enumerate(records):
        context = f"{path}: function[{index}]"
        if not isinstance(record, dict):
            raise ManifestError(f"{context}: expected a table")
        address = _format_address(record.get("address"), context)
        name = _symbol(record.get("name"), context)
        source = _require_string(record, "source", context)
        entry_mx = _require_string(record, "entry_mx", context)
        exit_mx = _require_string(record, "exit_mx", context)
        status = _require_string(record, "status", context)
        if entry_mx not in {"M0X0", "M0X1", "M1X0", "M1X1"}:
            raise ManifestError(f"{context}: invalid entry_mx {entry_mx!r}")
        if exit_mx not in {"M0X0", "M0X1", "M1X0", "M1X1"}:
            raise ManifestError(f"{context}: invalid exit_mx {exit_mx!r}")
        if status not in STATUSES:
            raise ManifestError(f"{context}: invalid status {status!r}")
        if address in addresses:
            raise ManifestError(f"{path}: duplicate function address {address}")
        if name in names:
            raise ManifestError(f"{path}: duplicate function symbol {name}")
        addresses.add(address)
        names.add(name)
        functions.append(Function(address, name, source, entry_mx, exit_mx, status))
    return sorted(functions, key=lambda item: item.address)


def load_bindings(path: Path) -> list[Binding]:
    document = _load_toml(path)
    if document.get("format") != 1:
        raise ManifestError(f"{path}: expected format = 1")
    records = document.get("binding", [])
    if not isinstance(records, list):
        raise ManifestError(f"{path}: binding must be an array of tables")

    bindings: list[Binding] = []
    addresses: set[str] = set()
    bridges: set[str] = set()
    for index, record in enumerate(records):
        context = f"{path}: binding[{index}]"
        if not isinstance(record, dict):
            raise ManifestError(f"{context}: expected a table")
        address = _format_address(record.get("address"), context)
        bridge = _symbol(record.get("bridge"), context)
        allow_draft = record.get("allow_draft", False)
        if not isinstance(allow_draft, bool):
            raise ManifestError(f"{context}: allow_draft must be a boolean")
        if address in addresses:
            raise ManifestError(f"{path}: duplicate binding address {address}")
        if bridge in bridges:
            raise ManifestError(f"{path}: duplicate bridge symbol {bridge}")
        addresses.add(address)
        bridges.add(bridge)
        bindings.append(Binding(address, bridge, allow_draft))
    return sorted(bindings, key=lambda item: item.address)


def load_cfg_files(cfg_dir: Path) -> tuple[list[CfgFile], dict[str, tuple[Path, str]]]:
    paths = sorted(cfg_dir.glob("*.cfg"), key=lambda path: path.name)
    if not paths:
        raise ManifestError(f"{cfg_dir}: no source cfg files found")

    cfg_files: list[CfgFile] = []
    banks: set[str] = set()
    hle: dict[str, tuple[Path, str]] = {}
    for path in paths:
        text = path.read_text(encoding="utf-8")
        bank_matches = [BANK_RE.fullmatch(line) for line in text.splitlines()]
        bank_values = [match.group(1).upper() for match in bank_matches if match]
        if len(bank_values) != 1:
            raise ManifestError(f"{path}: expected exactly one 'bank = XX' declaration")
        bank = bank_values[0]
        if bank in banks:
            raise ManifestError(f"{cfg_dir}: duplicate cfg bank {bank}")
        banks.add(bank)

        for line_number, line in enumerate(text.splitlines(), 1):
            stripped = line.strip()
            if not stripped or stripped.startswith("#"):
                continue
            if not stripped.startswith("hle_func"):
                continue
            match = HLE_RE.fullmatch(line)
            if not match or not PC16_RE.fullmatch(match.group(1)):
                raise ManifestError(f"{path}:{line_number}: malformed hle_func address")
            pc = match.group(1).upper()
            symbol = _symbol(match.group(2), f"{path}:{line_number}")
            address = f"{bank}:{pc}"
            if address in hle:
                raise ManifestError(f"{path}:{line_number}: duplicate hle_func {address}")
            hle[address] = (path, symbol)
        cfg_files.append(CfgFile(path, bank, text))
    return cfg_files, hle


def render_text_report(report: dict) -> str:
    selection_reasons = {
        "not_verified": "metadata status is not verified",
        "verified_not_integrated": "verified, but no consumer binding exists",
        "reference_only": "reference-only mode disables native replacements",
        "draft_not_enabled": "draft replacement is not enabled for runtime validation",
    }
    selected = report["selected"]
    fallbacks = [
        function
        for function in report["functions"]
        if function["selection"] != "selected"
    ]
    hand_hle = report["hand_authored_hle"]

    lines = [
        "Lufia II function source report",
        "================================",
        "",
        "Generated by the build from pinned decomp metadata and consumer bindings.",
        "Only verified and bound decomp functions can replace static recomp code.",
        "",
        f"Standalone decomp replacements used ({len(selected)}):",
    ]
    if selected:
        for function in selected:
            qualifier = (
                " [draft runtime validation]"
                if function["status"] == "draft"
                else ""
            )
            lines.append(
                f"- ${function['address']} {function['name']} "
                f"via {function['bridge']}{qualifier}"
            )
    else:
        lines.append("- None")

    lines.extend(
        [
            "",
            f"Static recomp fallbacks for decomp candidates ({len(fallbacks)}):",
        ]
    )
    if fallbacks:
        for function in fallbacks:
            reason = selection_reasons.get(
                function["selection"], function["selection"].replace("_", " ")
            )
            lines.append(
                f"- ${function['address']} {function['name']} "
                f"[{function['status']}]: {reason}"
            )
    else:
        lines.append("- None")

    lines.extend(
        [
            "",
            f"Consumer-owned HLE overrides ({len(hand_hle)}):",
        ]
    )
    if hand_hle:
        for function in hand_hle:
            lines.append(
                f"- ${function['address']} {function['symbol']} "
                "(not part of the standalone decomp)"
            )
    else:
        lines.append("- None")

    lines.extend(
        [
            "",
            "All other game functions use the generated static recomp/LLE path.",
            "",
        ]
    )
    return "\n".join(lines)


def generate(
    decomp_root: Path,
    bindings_path: Path,
    cfg_dir: Path,
    out_dir: Path,
    report_path: Path,
    reference_only: bool = False,
    text_report_path: Path | None = None,
    allow_draft: bool = False,
) -> dict:
    functions = load_functions(decomp_root / "metadata" / "functions.toml")
    bindings = load_bindings(bindings_path)
    cfg_files, hand_hle = load_cfg_files(cfg_dir)
    function_by_address = {item.address: item for item in functions}
    binding_by_address = {item.address: item for item in bindings}
    cfg_by_bank = {item.bank: item for item in cfg_files}

    selected: list[tuple[Function, Binding]] = []
    function_report: list[dict] = []
    for function in functions:
        binding = binding_by_address.get(function.address)
        is_selected = (
            binding is not None
            and not reference_only
            and (
                function.status == "verified"
                or (
                    allow_draft
                    and function.status == "draft"
                    and binding.allow_draft
                )
            )
        )
        if is_selected:
            if function.address in hand_hle:
                path, symbol = hand_hle[function.address]
                raise ManifestError(
                    f"{function.address}: selected replacement conflicts with "
                    f"hand-authored hle_func {symbol} in {path}"
                )
            bank = function.address[:2]
            if bank not in cfg_by_bank:
                raise ManifestError(
                    f"{function.address}: no source cfg declares bank {bank}"
                )
            selected.append((function, binding))

        if function.status == "verified" and binding is None:
            integration = "verified_not_integrated"
        elif reference_only:
            integration = "reference_only"
        elif (
            function.status == "draft"
            and binding is not None
            and binding.allow_draft
            and not allow_draft
        ):
            integration = "draft_not_enabled"
        elif function.status != "verified" and not is_selected:
            integration = "not_verified"
        else:
            integration = "selected"
        function_report.append(
            {
                "address": function.address,
                "binding": binding.bridge if binding else None,
                "name": function.name,
                "selection": integration,
                "status": function.status,
            }
        )

    generated_by_bank: dict[str, list[str]] = {}
    for function, binding in selected:
        generated_by_bank.setdefault(function.address[:2], []).append(
            f"hle_func {function.address[3:]} {binding.bridge}"
        )

    out_dir.mkdir(parents=True, exist_ok=True)
    for stale in sorted(out_dir.glob("*.cfg")):
        stale.unlink()
    for cfg in cfg_files:
        output = cfg.text
        additions = sorted(generated_by_bank.get(cfg.bank, []))
        if additions:
            output = output.rstrip() + "\n\n# Generated verified decomp replacements.\n"
            output += "\n".join(additions) + "\n"
        (out_dir / cfg.path.name).write_text(output, encoding="utf-8", newline="\n")

    unmatched = [
        {"address": item.address, "bridge": item.bridge}
        for item in bindings
        if item.address not in function_by_address
    ]
    report = {
        "format": 1,
        "functions": function_report,
        "hand_authored_hle": [
            {"address": address, "symbol": symbol}
            for address, (_, symbol) in sorted(hand_hle.items())
        ],
        "reference_only": reference_only,
        "selected": [
            {
                "address": function.address,
                "bridge": binding.bridge,
                "name": function.name,
                "status": function.status,
            }
            for function, binding in selected
        ],
        "unmatched_bindings": unmatched,
    }
    report_path.parent.mkdir(parents=True, exist_ok=True)
    report_path.write_text(
        json.dumps(report, indent=2, sort_keys=True) + "\n",
        encoding="utf-8",
        newline="\n",
    )
    if text_report_path is not None:
        text_report_path.parent.mkdir(parents=True, exist_ok=True)
        text_report_path.write_text(
            render_text_report(report), encoding="utf-8", newline="\n"
        )
    return report


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--decomp-root", type=Path, required=True)
    parser.add_argument("--bindings", type=Path, required=True)
    parser.add_argument("--cfg-dir", type=Path, required=True)
    parser.add_argument("--out-dir", type=Path, required=True)
    parser.add_argument("--report", type=Path, required=True)
    parser.add_argument("--text-report", type=Path)
    parser.add_argument("--reference-only", action="store_true")
    parser.add_argument("--allow-draft", action="store_true")
    args = parser.parse_args(argv)
    try:
        generate(
            args.decomp_root,
            args.bindings,
            args.cfg_dir,
            args.out_dir,
            args.report,
            args.reference_only,
            args.text_report,
            args.allow_draft,
        )
    except (ManifestError, OSError) as exc:
        print(f"decomp_manifest: error: {exc}", file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
