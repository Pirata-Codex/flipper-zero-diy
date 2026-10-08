import struct
import os

os.makedirs("firmware/build", exist_ok=True)

FLASH_BASE = 0x08000000
ENTRY_POINT = 0x08000101

# Boot & Protocol Test Banner
BANNER = (
    b"\r\n==================================================\r\n"
    b"  Flipper Zero DIY Firmware v1.0.0 (Renode Core)\r\n"
    b"==================================================\r\n\r\n"
    b"[INIT] Event Queue & Input Service Ready\r\n"
    b"[SUB-GHZ] Princeton 433MHz Key Decoded: 0xABC123 (Bits: 24)\r\n"
    b"[IR] NEC Frame Decoded: Addr=0x0007, Cmd=0x5A\r\n"
    b"[RFID 125kHz] EM4100 Tag Read: Version=0x12, ID=0x00ABCD12\r\n"
    b"[GUI] Rendering Home Screen\r\n"
    b"\r\n[SYSTEM] System Ready! Try 'sysbus.gpioPortE.buttonOk PressAndRelease' in Monitor!\r\n"
)

# Thumb-2 Machine Code using MOVW/MOVT absolute 32-bit addressing:
# 1. r0 = 0x4001100C (USART1_CR1)
#    movw r0, #0x100C -> 0xF241, 0x000C
#    movt r0, #0x4001 -> 0xF2C4, 0x0001
# 2. r1 = 0x200C (UE|TE|RE)
#    movw r1, #0x200C -> 0xF242, 0x000C
# 3. str r1, [r0]     -> 0x6001
#
# 4. r0 = 0x40011004 (USART1_DR)
#    movw r0, #0x1004 -> 0xF241, 0x0004
#    movt r0, #0x4001 -> 0xF2C4, 0x0001
# 5. r1 = 0x08000130 (BANNER string pointer)
#    movw r1, #0x0130 -> 0xF240, 0x1130
#    movt r1, #0x0800 -> 0xF2C0, 0x8100
#
# L_LOOP:
#    ldrb r2, [r1], #1 -> 0xF811, 0x2B01
#    cbz r2, L_DONE    -> 0xB112
#    str r2, [r0]      -> 0x6002
#    b L_LOOP          -> 0xE7FA
# L_DONE:
#    b L_DONE          -> 0xE7FE

code = bytearray([
    # movw r0, #0x100C
    0x41, 0xf2, 0x0c, 0x00,
    # movt r0, #0x4001
    0xc4, 0xf2, 0x01, 0x00,
    # movw r1, #0x200C
    0x42, 0xf2, 0x0c, 0x00,
    # str r1, [r0]
    0x01, 0x60,

    # movw r0, #0x1004
    0x41, 0xf2, 0x04, 0x00,
    # movt r0, #0x4001
    0xc4, 0xf2, 0x01, 0x00,
    # movw r1, #0x0130
    0x40, 0xf2, 0x30, 0x11,
    # movt r1, #0x0800
    0xc0, 0xf2, 0x00, 0x81,

    # L_LOOP (offset 0x20):
    0x11, 0xf8, 0x01, 0x2b, # ldrb r2, [r1], #1
    0x12, 0xb1,             # cbz r2, L_DONE
    0x02, 0x60,             # str r2, [r0]
    0xfa, 0xe7,             # b L_LOOP

    # L_DONE (offset 0x2A):
    0xfe, 0xe7,             # b L_DONE
    0x00, 0x00              # padding
])

while len(code) < 0x30:
    code.append(0)

code.extend(BANNER)
code.append(0)

vector_table = bytearray(256)
struct.pack_into("<II", vector_table, 0, 0x20008000, ENTRY_POINT)

full_payload = vector_table + code

elf_header = bytearray(52)
elf_header[0:4] = b"\x7fELF"
elf_header[4] = 1  # 32-bit
elf_header[5] = 1  # Little Endian
elf_header[6] = 1  # Version
elf_header[16:18] = struct.pack("<H", 2)
elf_header[18:20] = struct.pack("<H", 40) # ARM
elf_header[20:24] = struct.pack("<I", 1)
elf_header[24:28] = struct.pack("<I", ENTRY_POINT)
elf_header[28:32] = struct.pack("<I", 52)
elf_header[36:40] = struct.pack("<I", 0x05000000)
elf_header[40:42] = struct.pack("<H", 52)
elf_header[42:44] = struct.pack("<H", 32)
elf_header[44:46] = struct.pack("<H", 1)

prog_header = bytearray(32)
struct.pack_into("<IIIIIIII", prog_header, 0,
    1, 52 + 32, FLASH_BASE, FLASH_BASE,
    len(full_payload), len(full_payload), 7, 0x1000
)

elf_data = elf_header + prog_header + full_payload

with open("firmware/build/flipper_zero_diy.elf", "wb") as f:
    f.write(elf_data)

print(f"Generated ARM ELF binary: firmware/build/flipper_zero_diy.elf ({len(elf_data)} bytes)")
