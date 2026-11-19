#!/usr/bin/env python3
"""2K0300 SPI Flash 启动镜像打包脚本（占位实现）。

用法:
    python3 tools/mkimage.py blink.bin blink.flash.bin

当前行为: 对齐到 4KB 并输出。
TODO(V2/V4): 根据 S0 步骤的结论（dump 出厂 Flash 前 4KB 与官方 u-boot-spl.bin
对比）补齐 BootROM 要求的头部字段：
    - 若无头部校验: 本脚本维持现状即可;
    - 若需要头部:   在此填写 magic / 长度 / 校验和 / 签名等字段布局。
"""
import sys
from pathlib import Path

PAGE = 4096


def main() -> None:
    if len(sys.argv) != 3:
        print(__doc__)
        sys.exit(1)
    src, dst = Path(sys.argv[1]), Path(sys.argv[2])
    data = src.read_bytes()

    pad = (-len(data)) % PAGE
    out = data + b"\xff" * pad          # Flash 擦除态为 0xff

    # ==== TODO(V4): BootROM 头部占位，待 S0 结论后填充 ====
    # header = struct.pack("<IIII", MAGIC, src_offset, len(data), checksum)
    # out = header + out

    dst.write_bytes(out)
    print(f"{src} ({len(data)}B) -> {dst} ({len(out)}B, pad {pad}B)")
    print("注意: 头部字段尚未实现，详见脚本内 TODO(V4)。")


if __name__ == "__main__":
    main()
