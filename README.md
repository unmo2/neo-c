
# neo-c

This has Rerfference Count GC, and includes the generics collection libraries.

リファレンスカウントGCがありコレクションライブラリを備えてます。

version 1.0.3.30

neo-c supports freestanding 16-bit and 8-bit microcontroller targets through
`-micro16` and `-micro8`. The generated C uses a 16-bit pointer/`int` ABI with
a 32-bit `long`; `-micro8` additionally selects smaller heap and formatting
buffers for RAM-constrained 8-bit CPUs. AVR, MSP430, and Z80 builds are covered
by the `minux12`, `minux13`, and `minux14` execution tests.

neo-cは`-micro16`と`-micro8`により、標準Cライブラリを使わない16bit・8bit
マイコンに対応しています。生成Cは16bitポインタ、16bit `int`、32bit
`long`のABIを使用し、`-micro8`ではRAMの少ない8bit CPU向けにヒープと
書式化用バッファをさらに縮小します。AVR、MSP430、Z80でのビルドと実行は
`minux12`、`minux13`、`minux14`でテストできます。

PIC16F877AのようなHarvardアーキテクチャでは、SDCCの汎用ポインタが
コード・データなどのアドレス空間を識別するため24bitになります。この場合は
16bitフラットポインタ用の`-micro8`ではなく、ターゲット固有コードと
組み合わせる`-micro`を使用します。`minux15`ではneo-cからPIC用Intel HEXを
生成し、neo-c製PIC16エミュレータで`HELLO WORLD`を実行します。

## Small binaries

neo-c generates compact C output and links with section GC/LTO-friendly defaults.
On Fedora x86_64, `neo-c -bare a.nc` builds this Hello World program into a statically linked standalone Linux x86_64 executable of about 5K after `strip`, without linking libc.

neo-cは小さいCコードを生成し、不要な関数をリンク時に落としやすいので実行ファイルが小さくなります。
Fedora x86_64では、`neo-c -bare a.nc`でこのHello Worldをビルドすると、libcをリンクしない単体で動くLinux x86_64の静的リンク実行ファイルになり、`strip`後のサイズは約5Kです。

These sizes are measured after `strip`.

以下は`strip`後のサイズです。

| source | libc link mode | stripped size |
| --- | --- | ---: |
| `small/src/main.nc` | bare Linux x86_64, direct syscall `_start`, no libc/runtime | 174 bytes |
| `a.nc` | bare Linux x86_64, no libc, statically linked | 5K |
| `b.nc` | bare Linux x86_64, no libc, statically linked | 21K |
| `b.nc` | dynamic libc | 19K |
| libc Hello World sample | static libc | 711K |

For the smallest Linux x86_64 ELF, see `small/`. It emits a direct syscall
`_start` from neo-c source and uses `Neo.toml` `bare = true` plus
`strip_sections = true` to remove ELF section headers:

```sh
cd small
../cpm/cpm build
make size
```

Build command:

```sh
neo-c -bare a.nc
```

`a.nc`:

```c
#include <neo-c.h>

int main(int argc, char** argv)
{
    puts("HELLO WORLD");

    return 0;
}
```

`neo-c -bare b.nc` builds the richer `neo-c.h` sample as a standalone statically linked Linux x86_64 executable of about 21K after `strip`, without libc.
When linked dynamically, the same sample still produces about a 19K executable after `strip`.

`neo-c -bare b.nc`では、`neo-c.h`、文字列処理、オブジェクト初期化、生成された`to_string`まで使う少し大きいサンプルでも、libcなしの単体で動く静的リンクLinux x86_64実行ファイルが`strip`後で約21Kです。
同じサンプルを動的リンクすると、`strip`後の実行ファイルは約19Kです。

Build command:

```sh
neo-c -bare b.nc
```

`b.nc`:

```c
#include <neo-c.h>

struct sData
{
    int x;
    int y;
};

int main(int argc, char** argv)
{
    puts(s"HELLO WORLD" + new sData { x:111, y:222 }.to_string());
    return 0;
}
```

The static libc Hello World case is much larger because libc is linked into the executable, but it is still a single standalone binary.

静的リンクしたlibcのHello Worldは、libcを実行ファイルへ含めるので大きくなりますが、単体で動くバイナリになります。

```c
#include <stdio.h>

int main(int argc, char** argv)
{
    puts("HELLO WORLD");
    return 0;
}
```

# SAMPLE

``` C
#include <neo-c.h>

class sData
{
    int a;
    int b;
    
    void show()
    {
        printf("a %d b %d\n", self.a, self.b);
    }
};

class sData2 extends sData
{
    int c;
    
    void show()
    {
        printf("a %d b %d c %d\n", self.a, self.b, self.c);
    }
};

int main() 
{
    var data = new sData2 { a:123, b:234, c:345 };
    
    data.show();  // a 123 b 234 c 345
    
    puts("HO!" * 3);             // HO!HO!HO!\n

    return 0;
}
```


```
#include <neo-c.h>

int main(int argc,char** argv)
{
    "ABC".scan("."); // [A,B,C]
    "ABC".index_regex("b", -1, ignore_case:true); // 1
    "fooBAR".match("bar", ignore_case:true); // true
    "a1b2c3".split("\\d+"); // [a,b,c]
    "Hello".sub("l+", "L"); // HeLo
    "ABCABC".sub_block("ABC") { string("X") }; // XX
    "123 456 789".scan_block("[0-9][0-9][0-9]") { it.substring(0, 1) }; // [1,4,7]

    return 0;
}
```

neo-c outputs c source with standard C libraries only. So you can use this for micro computer or other system working c language.

With `-bare`, neo-c can also build without the standard C library on supported bare targets. On Linux x86_64 this uses `neo-c-libc.h` syscall wrappers and links with `-nostdlib`, so the executable is standalone and does not depend on libc.

`-bare`を使うと、対応しているbareターゲットでは標準Cライブラリなしでもビルドできます。Linux x86_64では`neo-c-libc.h`のsyscallラッパーを使い、`-nostdlib`でリンクするため、libcに依存しない単体実行ファイルになります。

## PICO

neo-c has two Pico / RP2040 paths.

`-pico` is the Pico SDK path. Use it when you want neo-c to include Pico SDK headers and build against the SDK environment.

`-micro32` is the recommended no-SDK bare-metal path for RP2040. It implies
`-micro` and `-bare`, defines `__BAREMETAL__`, `__NEO_MICRO__`, and
`__NEO_MICRO32__`, selects `neo-c-libc.h`, and verifies that the target C
compiler uses the 32-bit ILP32 data model.

neo-cにはPico / RP2040向けに2つの経路があります。

`-pico`はPico SDKを使う経路です。Pico SDKのヘッダを取り込み、SDK環境に対してビルドしたいときに使います。

`-micro32`はRP2040向けに推奨するSDKなしのbare-metal経路です。
`-micro`と`-bare`を含み、`__BAREMETAL__`、`__NEO_MICRO__`、
`__NEO_MICRO32__`を定義して`neo-c-libc.h`を選びます。また、ターゲットC
コンパイラが32bitのILP32データモデルであることをコンパイル時に検査します。

Example:

```sh
neo-c -micro32 -S utkernel.nc
arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -ffreestanding -fno-builtin \
    -fno-stack-protector -c -o utkernel.o utkernel.c
arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -ffreestanding -fno-builtin \
    -fno-stack-protector -nostdlib -nostartfiles -T link.ld \
    -o utkernel.elf utkernel.o -lgcc
```

`minux10` is a tested Pico-style sample. It contains a small Cortex-M0+
emulator written in neo-c and a minimal uT-Kernel subset generated with
`-micro32`.

`minux10`はテスト済みのPico風サンプルです。neo-cで書かれた小さな
Cortex-M0+エミュレータと、`-micro32`で生成する最小uT-Kernelサブセットを
含みます。

```sh
cd minux10
make test
```

## Microcontrollers

`-micro32` is the recommended mode for 32-bit non-Unix microcontrollers. It
supports the common ILP32 ABI used by ARM Cortex-M, RISC-V RV32, and 32-bit
ESP32 targets. It generates C only, links no standard C library, and expects
the final object or ELF to be built by the target toolchain.

`-micro16` targets small microcontrollers with 16-bit pointers and `int`, plus
a 32-bit `long`, such as the common MSP430 data model. It defines
`__NEO_MICRO16__`, uses 16-bit `size_t`, `ptrdiff_t`, `uintptr_t`, and
`intptr_t`, and maps `int32_t`/`uint32_t` to `long`/`unsigned long`.

`-micro8` targets 8-bit CPUs such as AVR. Standard C still requires `int` to
be at least 16-bit, and common 8-bit MCU toolchains use 16-bit pointers and
`int`, so this mode uses the same data ABI as `-micro16` while additionally
defining `__NEO_MICRO8__`. Its default heap is 512 bytes and its `vasprintf`
temporary stack buffer is 96 bytes.

`-micro32` fixes `size_t`, `ptrdiff_t`, `uintptr_t`, and `intptr_t` to 32-bit
types. A compiler whose pointers, `int`, or `long` are not 32-bit stops with a
compile error instead of producing a binary with an incompatible runtime ABI.
`-micro16` performs the corresponding checks for 16-bit pointers and `int`
with a 32-bit `long`.

Use `-micro` when implementing a different target ABI yourself. `-micro8`,
`-micro16`, and `-micro32` include all `-micro` behavior.

The user side only needs to provide `putchar(char)`. neo-c provides the rest of its small standard-C-compatible runtime from `neo-c-libc.h`, including `puts`, `printf`, `memset`, allocation support, `exit`, frame globals, and `uniq` runtime definitions. Programs without a `main` function are also valid with `-micro` and `-uniq`, which is useful for kernel or firmware objects.

The default `-micro32` heap is 64KB. The `-micro16` default is 1KB and its
`vasprintf` temporary stack buffer is reduced to 128 bytes. Override the heap
from the C compiler with `-DNEO_MICRO_HEAP_SIZE=...`.

Example for an AVR-style target:

```sh
neo-c -micro8 -S firmware.nc
avr-gcc -mmcu=atmega328p -std=gnu11 -Os -ffreestanding \
    -fno-builtin -fno-stack-protector -c firmware.c
```

The target project must still provide its startup code, vector table, linker
script, clock and peripheral initialization, and final link command. The
generated runtime uses GNU C extensions and is intended for GCC or Clang
compatible embedded toolchains.

| Target class | Status |
|---|---|
| ARM Cortex-M0/M0+/M3/M4/M7/M23/M33 | `-micro32`; tested by `minux10` |
| RISC-V RV32 | `-micro32` target |
| ESP32 Xtensa/RISC-V with a 32-bit GCC-compatible ABI | `-micro32` target |
| AVR and similar 8-bit CPUs with a 16-bit C ABI | `-micro8`; AVR ABI tested by `minux12`, 8KB SRAM collections by `minux16` |
| MSP430-style 16-bit CPU and pointer ABI | `-micro16`; tested by `minux13` |
| Z80 with SDCC's 16-bit pointer and `int` ABI | `-micro8`; tested by `minux14` |
| PIC16F877A and PIC targets with multiple address spaces | `-micro` plus target glue; tested by `minux15` |
| 64-bit embedded targets | Use `-micro` and a target-specific runtime |

`-micro32`は32bitの非Unixマイコン向けの推奨モードです。ARM Cortex-M、
RISC-V RV32、32bit ESP32などで一般的なILP32 ABIを対象にします。Cだけを
生成し、標準Cライブラリはリンクしません。最終的なオブジェクトやELFは
ターゲット用ツールチェインでビルドします。

`-micro16`は、ポインタと`int`が16bit、`long`が32bitの小型マイコンを
対象にします。代表的にはMSP430系のデータモデルです。
`__NEO_MICRO16__`を定義し、`size_t`、`ptrdiff_t`、`uintptr_t`、
`intptr_t`を16bit、`int32_t`と`uint32_t`を32bitの`long`型として生成します。

`-micro8`はAVRなどの8bit CPU向けです。標準Cの`int`は最低16bitであり、
一般的な8bit MCU用Cコンパイラもポインタと`int`に16bitを使うため、
データABIは`-micro16`と同じです。追加で`__NEO_MICRO8__`を定義し、
既定ヒープを512バイト、`vasprintf`の一時スタック領域を96バイトに
抑えます。

`size_t`、`ptrdiff_t`、`uintptr_t`、`intptr_t`を32bit型に固定し、
ポインタ、`int`、`long`が32bitではないコンパイラでは、互換性のない
バイナリを生成せずコンパイルエラーにします。異なるABIを独自実装する場合は`-micro`
を使用してください。`-micro8`、`-micro16`、`-micro32`には
`-micro`の全機能が含まれます。

ユーザー側で必要なのは`putchar(char)`の定義だけです。`puts`、`printf`、`memset`、メモリ確保、`exit`、フレーム管理用グローバル、`uniq`ランタイム定義などは`neo-c-libc.h`から生成Cへ入ります。`main`関数が無いカーネルやファームウェア用オブジェクトでも、`-micro`と`-uniq`で破綻しないようになっています。

`-micro32`の既定ヒープは64KBです。`-micro16`では既定ヒープを1KB、
`vasprintf`の一時スタック領域を128バイトに抑えています。ターゲットに
合わせる場合はCコンパイラ側で`-DNEO_MICRO_HEAP_SIZE=...`を指定してください。

### 8KB RAMでコレクションを使う

RAMが8KBあるマイコンでは、CPU ABI用オプションに`-micro-ram8k`を追加すると
`vector`、`list`、`map`を実用的な範囲で使用できます。

```sh
neo-c -micro8 -micro-ram8k -S firmware.nc
```

16bit CPUなら`-micro16 -micro-ram8k`、32bit CPUなら
`-micro32 -micro-ram8k`とします。特殊なポインタABIでは
`-micro -micro-ram8k`を使用できます。

このプロファイルは8KBのうち6KBをneo-cヒープに割り当て、残り約2KBを
スタック、グローバル変数、割り込み処理用として残します。さらに次の
省メモリ設定を有効にします。

- `vector`の初期容量を16要素から4要素へ縮小
- `map`の初期テーブルを128要素から16要素へ縮小
- `map`は容量不足時に2倍、`vector`は1.5倍ずつ拡張
- GC割り当てヘッダから型名、ソース位置、割り当て時履歴を除去
- `vasprintf`の一時スタック領域を128バイトに制限
- 8bit/16bit ABIのmallocアラインメントを2バイトへ縮小

現在の回帰テストでは、6KB固定ヒープ内で`vector<int>` 128要素、
`list<int>` 12要素、`map<int,int>` 6要素を同時に作成し、追加、検索、
集計、参照カウントによる解放まで実行しています。要素型や一時オブジェクトに
よって使用量は変わるため、大きな文字列や複数の大型mapを保持する場合は
アプリケーション側で上限を決めてください。

`minux16`ではSRAMがちょうど8KBのATmega640用ELFを生成し、neo-c製AVR
エミュレータで同じ`vector`、`list`、`map`試験を実行します。AVR GCCは
通常の文字列定数もSRAMへ配置するため、この実機向けイメージではヒープを
4KBへ上書きし、`.data`、`.bss`、スタック用の領域を確保しています。

```sh
cd minux16
make test
```

ヒープ量は引き続き上書きできます。

```sh
target-gcc -DNEO_MICRO_HEAP_SIZE=5632 ...
```

`-micro-ram8k`では割り当て時の詳細な型名・ソース位置は保存しませんが、
panic時の現在のstackframe表示、参照カウント、生存チェック、境界チェックは
維持されます。メモリ不足時は不正アドレスを参照せず`out of memory`で停止します。

ターゲット側ではstartupコード、割り込みベクタ、リンカースクリプト、
クロックと周辺機器の初期化、最終リンク設定が別途必要です。生成ランタイムは
GNU C拡張を使うため、GCCまたはClang互換の組み込み向けツールチェインを
使用します。特殊なポインタ幅や複数アドレス空間を使うPICなどでは、
ターゲット固有のランタイム調整が必要になる場合があります。

### CPU別の使い方

neo-cはマイコン用の機械語を直接生成せず、対象ABIに合わせたCを生成します。
最初にneo-cで`.nc`を`.c`へ変換し、その後は各CPUのクロスコンパイラ、
startupコード、リンカースクリプトを使ってオブジェクトまたはELFを作ります。

ユーザー側では最低限、対象機のUARTなどへ1文字出力する`putchar`を定義します。

```c
#include <neo-c.h>

void putchar(char c)
{
    volatile unsigned char* uart = (volatile unsigned char*)UART_TX_ADDRESS;
    *uart = (unsigned char)c;
}
```

`main`がないファームウェアでは、任意のエントリ関数をstartupコードから
呼び出せます。

```c
int firmware_entry()
{
    puts("HELLO WORLD");
    return 0;
}
```

#### ARM Cortex-M / Raspberry Pi Pico

Cortex-MとRP2040は32bitポインタ、32bit `int`、32bit `long`のILP32 ABI
なので`-micro32`を使用します。

```sh
neo-c -micro32 -S firmware.nc
arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -Os -ffreestanding \
    -fno-builtin -fno-stack-protector -c firmware.c -o firmware.o
arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -nostdlib -nostartfiles \
    -T link.ld -Wl,--gc-sections -o firmware.elf startup.o firmware.o -lgcc
```

Cortex-M3/M4/M7などでは`-mcpu=`を対象CPUへ変更し、必要ならFPU関連の
`-mfpu=`と`-mfloat-abi=`も指定します。RP2040/uT-Kernelの実行例は
`minux10`で確認できます。

```sh
cd minux10
make test
```

#### RISC-V RV32

RV32もILP32 ABIなので`-micro32`を使用します。CPUに応じて`-march`と
`-mabi`を変更してください。

```sh
neo-c -micro32 -S firmware.nc
riscv64-linux-gnu-gcc -march=rv32imac -mabi=ilp32 -Os -ffreestanding \
    -fno-builtin -fno-stack-protector -c firmware.c -o firmware.o
riscv64-linux-gnu-gcc -march=rv32imac -mabi=ilp32 -nostdlib -nostartfiles \
    -T link.ld -Wl,--gc-sections -o firmware.elf startup.o firmware.o -lgcc
```

ハードウェア浮動小数点を使う場合は、CPUとツールチェインに合わせて
`rv32imafc`/`ilp32f`などの組み合わせへ変更します。

#### ESP32

32bitのESP32 XtensaおよびESP32-C3などのRV32製品では`-micro32`を
使用します。生成したCをESP-IDFのcomponentへ追加し、ESP-IDF側でstartup、
フラッシュ配置、割り込み、UART初期化を行います。

```sh
neo-c -micro32 -S firmware.nc
```

ESP32 Xtensaでは`xtensa-esp32-elf-gcc`、ESP32-C3/C6などでは
`riscv32-esp-elf-gcc`が使用されます。通常はコンパイラを直接呼ばず、
`idf.py build`から生成Cをビルドします。`putchar`はESP-IDFのUART APIまたは
対象UARTレジスタへ接続してください。

#### AVR ATmega

AVRは8bit CPUですが、通常のAVR GCC ABIではポインタと`int`が16bit、
`long`が32bitです。通常はCPU幅を明示する`-micro8`を使用します。

```sh
neo-c -micro8 -S firmware.nc
avr-gcc -mmcu=atmega328p -std=gnu11 -Os -ffreestanding \
    -fno-builtin -fno-stack-protector -c firmware.c -o firmware.o
avr-gcc -mmcu=atmega328p -nostdlib -Wl,--gc-sections \
    -o firmware.elf startup.o firmware.o -lgcc
```

`minux12`はAVRの16bit C ABIそのものを検証する目的で、意図的に
`-micro16`を使用しています。`-micro8`と`-micro16`はAVR上で同じ型幅を
検証しますが、`-micro8`はヒープと書式化用スタックをさらに小さくします。

```sh
cd minux12
make test
```

#### MSP430

MSP430はCPU、ポインタ、`int`が16bitで、`long`が32bitなので
`-micro16`を使用します。ClangにMSP430バックエンドが含まれる環境では
次のようにビルドできます。

```sh
neo-c -micro16 -S firmware.nc
clang --target=msp430 -mmcu=msp430g2553 -Oz -ffreestanding \
    -fno-builtin -fno-stack-protector -c firmware.c -o firmware.o
ld.lld -T link.ld -e firmware_entry -o firmware.elf firmware.o
```

MSP430G2553のELF生成とneo-c製エミュレータでの実行は`minux13`にあります。

```sh
cd minux13
make test
```

#### Zilog Z80

Z80は8bit CPU、16bitアドレス空間で、SDCCのZ80 ABIは16bitポインタと
16bit `int`を使うため`-micro8`を使用します。

```sh
neo-c -micro8 -S firmware.nc
sdcc-sdcc -mz80 --std-c11 --opt-code-size --no-std-crt0 \
    --code-loc 0x0100 --data-loc 0x8000 \
    '-D__builtin_va_list=void *' firmware.c -o firmware.ihx
```

SDCCでは、最小ファームウェアが可変長引数を使わない場合でも、neo-cが生成する
型定義を解釈させるため`__builtin_va_list`の互換定義が必要です。
Intel HEX生成、neo-c製Z80エミュレータ、`LOCATE`を含むncurses全画面の
MSX-BASIC風IDEは`minux14`にあります。

```sh
cd minux14
make test
make ide
```

#### その他のCPU

次の条件を確認してオプションを選択します。

| C ABI | neo-c option |
|---|---|
| 32bit pointer / 32bit `int` / 32bit `long` | `-micro32` |
| 16bit pointer / 16bit `int` / 32bit `long` on a 16bit CPU | `-micro16` |
| 16bit pointer / 16bit `int` / 32bit `long` on an 8bit CPU | `-micro8` |
| Other data model | `-micro` with a target-specific runtime |

PICの複数アドレス空間、banked memory、24bitポインタなど、通常のフラットな
CポインタモデルではないCPUには追加対応が必要です。RAMが小さい場合は
`NEO_MICRO_HEAP_SIZE`も対象に合わせて縮小します。

```sh
target-gcc -DNEO_MICRO_HEAP_SIZE=256 ...
```

#### Microchip PIC16F877A

PIC16F877Aは8bitデータパスと14bit命令語を持ちますが、SDCCの汎用ポインタは
Harvardアーキテクチャのアドレス空間を識別するため24bitです。そのため、
16bitのフラットポインタを要求する`-micro8`ではなく、PIC固有ランタイムと
組み合わせる`-micro`を使用します。

```sh
neo-c -micro -S firmware.nc
sdcc-sdcc -mpic14 -p16f877a --std-c11 --opt-code-size \
    --no-warn-non-free -c firmware.c -o firmware.o
```

PIC側ではstartup、リンカースクリプト、SFR定義、UART初期化などが必要です。
PIC16F877A向けIntel HEXの生成と、neo-c製PICエミュレータによる
`HELLO WORLD`の実行テストは`minux15`にあります。

```sh
cd minux15
make test
```

## minux9

`minux9` is a small Unix-like operating system for RISC-V made with neo-c.
The kernel and several userland commands are written in neo-c or built from C sources generated by neo-c.
It includes a shell, `vi`, `cc`, `as`, `ld`, `cpp`, file utilities, and a tiny filesystem image, and it can boot under QEMU or the bundled RISC-V emulator.

`minux9`はneo-cで作ったRISC-V向けの小さなUnix-like OSです。
カーネルと複数のユーザーランドコマンドはneo-c、またはneo-cが生成したCソースからビルドされています。
shell、`vi`、`cc`、`as`、`ld`、`cpp`、ファイル操作コマンド、小さなファイルシステムイメージを含み、QEMUまたは同梱のRISC-Vエミュレータ上で起動できます。

The easiest way to install required packages, build the kernel/userland, create the filesystem image, and boot it is:

必要なパッケージの導入、kernel/userlandのビルド、filesystem image作成、起動までまとめて行うには以下を実行します。

```sh
cd minux9
sh fast_build.sh
```

For a manual build after dependencies are installed:

依存パッケージ導入済みの環境で手動ビルドする場合:

```sh
cd minux9
make clean
make
make run
```

On Debian/Ubuntu/Fedora, the top-level `install_pkg.sh` installs the RISC-V cross compiler and QEMU packages used by minux9.
If the system has `riscv64-linux-gnu-gcc`, use the Linux GNU cross toolchain settings explicitly:

Debian/Ubuntu/Fedoraでは、トップレベルの`install_pkg.sh`がminux9で使うRISC-VクロスコンパイラとQEMUパッケージもインストールします。
`riscv64-linux-gnu-gcc`がある環境では、Linux GNUクロスツールチェイン設定を明示できます。

```sh
cd minux9
make CCPREFIX=riscv64-linux-gnu- RISCV_ABI=lp64d
make run CCPREFIX=riscv64-linux-gnu- RISCV_ABI=lp64d
```

Regression tests can be run with:

回帰テストは以下で実行できます。

```sh
cd minux9
make test
make test-neoc-hello
```

# インストール

sh fast_build.sh will automatically install the necessary packages.

Supports Linux, MacOS (Darwin), raspberry pi, and baremetal.

sh fast_build.shとすると自動的に必要なパッケージがインストールされます。

LinuxとMacOS(Darwin), raspberry pi, ベアメタルをサポートしています。

`install_pkg.sh` also installs the RISC-V cross compiler and QEMU packages used by minux9 on Fedora, Debian, and Ubuntu.

`install_pkg.sh` はFedora、Debian、Ubuntuでminux9用のRISC-VクロスコンパイラとQEMUパッケージもインストールします。

sudoとgitは事前にインストールしてください。

Please install sudo and git before the build.

``` 
git clone https://github.com/ab25cq/neo-c
cd neo-c
sh fast_build.sh
```

`clean-build.sh` and `clean-self-host.sh` have been removed. Use
`fast_build.sh` for the normal install path, `make clean && sh fast_build.sh`
when you want to rebuild from a clean tree, or `cpm` for package-style project
builds.

`self-host.sh` and `fast_build.sh` use parallel build by default.
`fast_build.sh` also builds `target/debug/ncc` through cpm before install, so
the cpm-managed compiler path is checked during normal install workflows. With
`-lowmem`, it builds `cpm` itself but skips the extra cpm-managed `ncc`
verification to keep memory use down.

If you don't have enough memory, you can use below to install neo-c.

```
git clone https://github.com/ab25cq/neo-c
cd neo-c
sh fast_build.sh -lowmem
```

`clean-build.sh` と `clean-self-host.sh` は削除されました。通常の install
には `fast_build.sh`、clean してからビルドしたい場合は
`make clean && sh fast_build.sh`、プロジェクト単位のビルドには `cpm` を
使ってください。

If you pass `-lowmem`, the build scripts enable low-memory mode and disable parallel build.
With `-lowmem`, neo-c can build even on systems with 512MB of memory.

`self-host.sh` と `fast_build.sh` はデフォルトで並列ビルドします。
`fast_build.sh` は install 前に cpm 経由の `target/debug/ncc` もビルドして、
cpm 管理のコンパイラ経路も確認します。`-lowmem` の場合はメモリ消費を
抑えるため、`cpm` 本体だけをビルドし、追加の cpm 経由 `ncc` 検証は省略します。

`-lowmem` を付けると low-memory mode を有効にし、並列ビルドを無効にします。
`-lowmem` を使うと、512MBのメモリを持つシステムでもneo-cをビルドできます。

If generated C gets too large to compile on a low-memory system, use `-lowmem` when transpiling your `.nc` file. In this mode, neo-c stops auto-generating helper methods such as `equals`, `operator_equals`, `operator_not_equals`, `to_string`, `compare`, and `get_hash_key`, so you may need to define them explicitly.

```
ncc -lowmem -c your_source.nc
```

You can also enable it for project builds:

```
LOWMEM=1 make -j1 self-host
sh self-host.sh -lowmem
sh fast_build.sh -lowmem
```

## libc-free self-host

On Linux x86_64, you can self-host neo-c without the standard C library by passing `BARE=1` to make:

```sh
make BARE=1 self-host
make BARE=1 ncc
```

You can also run the packaged script after a normal install build:

```sh
sh fast_build.sh
sh bare-self-host.sh
```

`BARE=1` passes `-bare` to the self-host transpiler, avoids libc-only paths such as `open_memstream`, and links `ncc` with `-nostdlib -Wl,-e,_start`. The `-bare` option writes `__BAREMETAL__` into generated C source by itself, so the Makefile does not need to add `-D__BAREMETAL__`. The resulting `ncc` is a stripped, statically linked executable that uses `neo-c-libc.h` instead of libc.

Linux x86_64では、以下のように標準Cライブラリを使わずにneo-cをセルフホストできます。

```sh
make BARE=1 self-host
make BARE=1 ncc
```

通常の install ビルド後に、同梱スクリプトでも実行できます。

```sh
sh fast_build.sh
sh bare-self-host.sh
```

`BARE=1`はセルフホスト時のトランスパイルに`-bare`を渡し、`open_memstream`のようなlibc専用の経路を避け、`ncc`を`-nostdlib -Wl,-e,_start`でリンクします。`-bare`オプションだけで生成Cソースへ`__BAREMETAL__`を書き込むため、Makefile側で`-D__BAREMETAL__`を追加する必要はありません。生成される`ncc`はstrip済みの静的リンク実行ファイルで、libcではなく`neo-c-libc.h`を使います。

For `vasprintf`, x86 and x86_64 keep a larger temporary stack buffer for self-host code generation. Other architectures use a smaller buffer to reduce stack pressure on small bare-metal systems such as Pico.

`vasprintf`は、x86/x86_64ではセルフホストのコード生成向けに大きめの一時スタックバッファを使います。それ以外のアーキテクチャでは、Picoのような小さいbare-metal環境でスタック消費を抑えるため小さいバッファを使います。

To install a vi clone called vin, a string processing interpreter called zed,
a console filer called mf, an original shell called shsh, and the neo-c package
manager cpm, do the following:

vinというviクローン、zedという文字列処理インタプリタ、mfというコンソールファイラ、
shshというオリジナルのシェル、neo-c用パッケージマネージャーcpmを
インストールするには以下のようにします。

```
sh all_build.sh
```

## cpm package manager

`cpm` is a small Cargo-like package manager for neo-c projects. It was ported
from the `../c-` package manager and uses `Neo.toml` plus `.nc` sources.

```sh
cpm new hello
cd hello
cpm build
cpm run
cpm test
cpm val
cpm leak
cpm clean
```

`cpm new hello` creates `Neo.toml`, `lib/`, `src/common.h`, `src/main.nc`, and
`.gitignore`.
`lib/` contains project-local copies of `neo-c.h`, `neo-c-str.nc`,
`neo-c-str.h`, `neo-c-libc.h`, `neo-c-net.h`, and `neo-c-pthread.h`.
`cpm build` compiles `lib/neo-c-str.nc` into `target/debug/neo-c-str.c` and
`target/debug/neo-c-str.o`, then links that project-local object.
`cpm build` compiles every `.nc` file under `src` to generated C and objects
under `target/debug`, then links `target/debug/<package-name>` with `neo-c`.
When `src/common.h` exists, cpm automatically includes it at the `.nc` source
stage before every `src/*.nc` file. Put declarations shared by files under
`src/` there. The generated template includes `<neo-c.h>` in `src/common.h`,
so declarations can use neo-c aliases such as `string`.
`cpm build` runs source transpile/compile jobs in parallel by default and keeps
the final link step serial. Use `CPM_JOBS=1 cpm build` or `[build] jobs = 1`
for a serial build, or set a larger value such as `CPM_JOBS=4` to choose the
parallelism explicitly. If generated C basenames would collide, cpm falls back
to the serial path.
For low-memory systems, use `CPM_LOWMEM=1 cpm build` or `[build] lowmem = true`.
This forces serial builds and passes `-lowmem` to neo-c, which is intended for
512MB-class machines.
`strip` defaults to `false` in cpm projects; set `[build] strip = true` only
when you want cpm to strip the final binary after linking.
Legacy source layouts can set `sources = "src/main.nc src/file1.nc"` in `Neo.toml` to
build an explicit ordered source list. `cpm install` installs the built binary
to `$DESTDIR/bin`, defaulting to `/usr/local/bin`.
The top-level neo-c compiler now also has `Neo.toml`; run `make cpm-build-ncc`
or `CPM_NEOC=./neo-c cpm/cpm build` to build `target/debug/ncc` through cpm.
The top-level compiler `.nc` sources live under `src/`, while checked-in
self-host generated C lives under `c-src/` so `make ncc` can still bootstrap
from C without an existing `ncc`. Test-like `test_*` sources are kept under
`code/`.
The existing Makefile remains the bootstrap, self-host, bare, low-memory, and
PGO build path.
To build this repository with cpm after a normal bootstrap compiler exists:

```sh
make cpm-build-ncc
```

The equivalent direct command is:

```sh
make -C cpm
CPM_NEOC=./neo-c cpm/cpm build
```

For low-memory systems, use the cpm lowmem path. It runs serially and passes
`-lowmem` only to the compile/transpile steps:

```sh
CPM_LOWMEM=1 make cpm-build-ncc
```

The cpm build writes generated C, objects, and the compiler binary under
`target/debug/`; it does not replace the checked-in self-host C sources in the
repository root.

The bundled `vin`, `zed`, `mf`, `shsh`, `cinatora`, `webweb`, and `webweb/dbdb`
tools are cpm projects with local `lib/` standard-library snapshots, so their
generated C and objects are kept under `target/debug`.
Set `CPM_NEOC=/path/to/neo-c` to use a specific compiler binary. Set
`CPM_STDLIB_DIR=/path/to/neo-c` if `cpm new` cannot find the standard library
sources.

`make install` installs `neo-c`, `ncc`, and `cpm` under `DESTDIR/bin`, and
installs the project template standard library sources under
`DESTDIR/share/neo-c`. Header files are also installed under
`DESTDIR/include` for existing projects that include them directly. Runtime
objects and `neo-c-str.nc` are not installed under `/usr/local/lib`; new `cpm`
projects copy their own snapshots from `share/neo-c` into `lib/`.

`cpm`は`../c-`のパッケージマネージャーをneo-c向けに移植したものです。
`Neo.toml`と`src/*.nc`を使い、`build`、`run`、`test`、`val`、`leak`、
`clean`を提供します。

to build fastest binary

```
sh build-fastest.sh
```

`webweb` is a good reference if you want a general web server in neo-c.
It serves static files, runs CGI, supports HTTP and HTTPS, and has a small database server example with `dbdb`.

neo-cで一般的なWebサーバーを書きたいなら`webweb`が良いサンプルです。
静的ファイル配信、CGI実行、HTTP/HTTPS対応、`dbdb`を使った小さなDBサーバー例まで入っています。

See [/home/ab25cq/neo-c/webweb/README.md](/home/ab25cq/neo-c/webweb/README.md) for quick start and usage examples.

## msxide

`msxide` is an MSX/Z80 emulator workbench ported from the c- project to neo-c.
It builds with `cpm`, uses ncurses for a fullscreen MSX-BASIC-like text screen,
and includes a clean-room mock BASIC ROM generator for smoke tests.

```sh
cd msxide
make test
target/debug/msxide
```

# Histories

```
1.0.3.30 Added cpm parallel transpile/compile builds with `CPM_JOBS` or `[build] jobs`, plus cpm low-memory mode with `CPM_LOWMEM=1` or `[build] lowmem = true`. Low-memory cpm builds force serial jobs and pass `-lowmem` only to neo-c compile/transpile steps, leaving the final link step clean; this keeps cpm usable on 512MB-class systems. cpm now defaults `strip` to `false`; set `[build] strip = true` explicitly when a stripped final binary is wanted. Moved the top-level compiler `.nc` sources under `src/`, moved generated self-host C under `c-src/`, and keeps `test_*` files under `code/` while preserving C-only bootstrap builds. Documented `CPM_LOWMEM=1 make cpm-build-ncc` for building the top-level compiler through cpm in low-memory mode.
1.0.3.29 Ported the `../c-` package manager as `cpm` for neo-c projects. `cpm` now creates `Neo.toml`/`lib`/`src/main.nc`, copies the neo-c standard library source into each project, builds `lib/neo-c-str.nc` into `target/debug/neo-c-str.o`, builds all `.nc` files under `src` with `neo-c`, supports `new`, `init`, `build`, `run`, `test`, `val`, `leak`, `clean`, and `install`, and is included in `all_build.sh`. `cpm` also supports explicit ordered `sources` lists for legacy multi-file layouts, preserves existing generated C files while moving new generated C under `target/debug`, and builds `neo-c-str.nc` with `-uniq` when it appears in `sources`. The top-level neo-c compiler now has `Neo.toml` and can build `target/debug/ncc` with `make cpm-build-ncc`; `clean-self-host.sh` was removed; `fast_build.sh` runs that cpm-managed compiler build before install, while `-lowmem` skips only the extra cpm `ncc` verification. The existing Makefile remains the bootstrap/self-host/bare/low-memory/PGO path. `vin`, `zed`, `mf`, `shsh`, `cinatora`, `webweb`, and `webweb/dbdb` now build and install through `cpm` with project-local `lib/` runtime snapshots instead of their old Makefile paths. `webweb` keeps its CGI Makefile while the main server, `dbdb`, and `dbdb/client` place generated C, objects, and binaries under `target/debug`. `make install` installs `neo-c`, `ncc`, and `cpm` under `DESTDIR/bin`, installs project template standard library sources under `DESTDIR/share/neo-c`, and installs headers under `DESTDIR/include` for existing projects; runtime objects and `neo-c-str.nc` are still not installed to `/usr/local/lib`. Linux/macOS libc builds now use `execinfo` native backtraces on panic, and `neo-c -g` keeps debug info plus `.nc` `#line` mappings for source file and line output. Ported the c- `msxide` MSX/Z80 emulator workbench to neo-c with `cpm` build support, ncurses UI, mock BASIC ROM generation, and self/ROM/BASIC/program/list/command/ASM tests. Added `cpm` `bare`, `cc`, `cflags`, `linker`, `linker_flags`, `linker_script`, and `strip_sections` build settings plus a `small` project that builds a 174-byte Linux x86_64 bare ELF with `cpm build`.
1.0.3.28 Added `-memleak-stacktrace` as an optional allocation-call-stack mode while keeping leak detection enabled by default. Reduced `vector` growth to 1.5x and standardized `map` growth to 2x, then documented the `-micro-ram8k` profile and added `minux16`, which runs `vector`/`list`/`map` on an emulated 8KB-SRAM ATmega640.
1.0.3.27 Added `minux15`, a PIC16F877A target test. neo-c generates freestanding C with `-micro`, SDCC and gputils produce a real PIC Intel HEX image, and the neo-c-written PIC16 emulator verifies `HELLO WORLD` through the emulated `TXREG`. Documented why PIC uses `-micro` instead of `-micro8`: SDCC generic pointers are three bytes because they encode the Harvard address space. Added PIC startup code, target-specific UART glue, banked file-register handling, and an eight-level hardware return stack model.
1.0.3.26 Added `-micro16` and `-micro8` freestanding target modes. `-micro16` supports a 16-bit pointer/`int` ABI with a 32-bit `long`; `-micro8` uses the same C ABI while reducing the default heap and formatting stack buffers for 8-bit CPUs. Added AVR, MSP430, and Z80 Hello World execution tests in `minux12`, `minux13`, and `minux14`. `minux14` also includes an ncurses 40x24 full-screen MSX-BASIC-style IDE with `LOCATE`, compound statements, line editing, history, and function keys.
1.0.3.25 Added `-micro` as a non-Unix microcontroller source-generation mode. `-micro` now implies `-bare`, emits `__BAREMETAL__` and `__NEO_MICRO__`, uses `neo-c-libc.h` without Unix file/syscall APIs, requires only user-provided `putchar(char)`, and emits `uniq` runtime definitions even when the source has no `main`. Updated the Pico documentation and verified `minux10` with `make -B test`.
1.0.3.24 Added `bare-self-host.sh`, documented the `clean-self-host.sh` to `bare-self-host.sh` workflow, and kept the checked-in self-host C sources in normal libc-generated form. `make clean` also removes the old generated `self-host` file.
1.0.3.23 `fast_build.sh` now detects self-host C sources generated with `-bare`, rebuilds a temporary bare `ncc`, regenerates normal libc self-host sources, and then continues the normal install build; `ccpp.c` generation no longer emits duplicate typedefs because `ccpp_body.h` owns its libc/bare declarations.
1.0.3.22 Fixed Linux x86_64 bare self-host file creation by using Linux `O_CREAT`/`O_TRUNC`/`O_APPEND` values in `neo-c-libc.h`; bare `ncc` can now generate preprocessor output files such as `a.nc.i` during libc-free builds.
1.0.3.21 Rechecked bare builds on darwin.
1.0.3.20 Regenerated the checked-in self-host C sources in normal libc mode so regular Fedora/Linux self-host builds no longer link duplicate bare-runtime symbols such as `brk` and `__atexit_funcs`.
1.0.3.19 Darwin bare builds now use `-Wl,-dead_strip`, disable stack protectors, link through libSystem with `_main`, and avoid leaking Darwin SDK typedefs into generated bare C. macOS bare self-host builds with `make BARE=1 self-host` and `make BARE=1 ncc` now pass, and `neo-c -bare a.nc` links and runs on Darwin.
1.0.3.18 Generated C now emits explicit parent-class pointer casts for inherited method calls, fixing strict Clang/Arch self-host builds. Runtime / and % zero checks now leave sizeof-based declaration constants as valid C constant expressions. Revalidated clean self-host plus bundled subproject builds, including cinatora, webweb, vin, and minux9.
1.0.3.17 ccpp now uses normal libc headers unless `BARE=1`; generated panic checks no longer redeclare `puts`, which improves freestanding targets such as minux9. Fedora/Debian/Ubuntu package setup now covers the RISC-V toolchain and QEMU used by minux9, and minux9 shell commands were verified under QEMU.
1.0.3.16 `-bare` now writes `__BAREMETAL__` into generated C source by itself, so Makefile no longer adds `-D__BAREMETAL__`; ccpp no longer depends on standard C headers.
1.0.3.15 Document libc-free Linux x86_64 self-host builds with `make BARE=1 self-host` and `make BARE=1 ncc`; `vasprintf` now uses a large stack buffer only on x86/x86_64 and a small one elsewhere.
1.0.3.14 Linux x86_64 `-bare` implements read/write/open/close based file I/O without libc and fixes `_start` stack alignment so exception stackframes do not segfault.
1.0.3.13 Linux x86_64 `-bare` can build libc-free standalone statically linked binaries; document 5K `a.nc` and 21K `b.nc` stripped sizes.
1.0.3.12 Array bounds checks and runtime / and % by zero panics now print stackframe output before exiting.
1.0.3.11 Fixed-size C array [] loads and [] assignments now check known bounds at runtime and panic with exit(2) on out-of-range indexes.
1.0.3.10 Runtime / and % now check the right operand every time and panic with exit(2) on division or modulo by zero.
1.0.3.9 list/vector/map [] followed by ! now uses optional access and panics on out-of-range indexes or missing keys.
1.0.3.8 FILE*::fread, FILE*::fclose, FILE*::readlines, socket_fd::write, and client_socket2 now return Result<T>. Updated bundled subprojects for the new Result-returning APIs.
1.0.3.7 Document the file API spec change: FILE*::read and FILE*::write are removed; use FILE*::fread and FILE*::fwrite instead.
1.0.3.6 Document Result<T> as the standard result API and make the removal of RESULT(T), SOME, and NONE explicit.
1.0.3.5 Result<T> is now the standard Some/None payload enum result type. The old tuple2-based RESULT(T), SOME, and NONE macros are removed.
1.0.3.4 optional collection access now uses Result<T> instead of neo_option<T>.
1.0.3.3 list/vector/map [] can use .catch as optional access sugar: Some unwraps to the value, None runs the catch block and returns its result.
1.0.3.2 list/vector/map [] can be matched as Some/None inside .case while normal [] access keeps the old zero-clear fallback.
1.0.3.1 payload enum Result<T,E> works with ?? propagation. nested functions now report an error. STDC FENV_ACCESS pragma is preserved in generated C.
1.0.3.0 Makefile strips the built ncc binary by default.
1.0.2.9 neo-c driver strips linked output executables by default.
1.0.2.8 document stripped small binary sizes for dynamic and static libc examples.
1.0.2.7 RESULT(T) catch ownership bug fixed. catch can ignore an error by assigning a default value to Value.v1.
1.0.2.6 span,ref,optional to local variable bug fixed.
1.0.2.5 optimize binary size
1.0.2.4 Removed Result template.
1.0.2.3 template bug was fixed.
1.0.2.2 template bug was fixed.
1.0.2.1 vector bug was fixed.
1.0.2.0 normalize effective pointer depth in auto-generated equals/clone/finalize, fixing multi-level pointer and typedef alias cases used by remove().
1.0.1.9 document `??` propagation for `Result<T>` / `RESULT(T)`.
1.0.1.8 fix anonymous enum regressions around payload enum parsing and self-hosted comelang compilation.
1.0.1.7 add Rust-like payload enum syntax with generated variant constructors and predicate/getter methods.
1.0.1.5 support both RESULT(T) and Result<T>, and allow ! unwrap on both.
1.0.1.4 rewrite neo-c-net.h manual to use begin/end/next loop style like webweb.
1.0.1.3 add quick start for webweb.
1.0.1.2 recommend webweb as a general web server.
1.0.1.1 document neo-c-str.h.
1.0.1.0 document neo-c-net.h zero cost iterator API.
1.0.0.9 optimize more faster.
1.0.0.8 optimize more faster.
1.0.0.7 optimize more faster.
1.0.0.6 optimize more faster.
1.0.0.5 can build in 512M memory system. may be build in radpberry pi zero.
1.0.0.3 more improvement.
1.0.0.2 optimize enabled.
1.0.0.1 checked alpine on arm and amd64. all ok
1.0.0.0 I can't continue to deep docus on this system. I sometimes to use this to check the functions and to be more better than to be perfect.
0.9.2.9 checked arm and x86_64 on linux and mac. If you don't have enough memory, you can use sh fast_build.sh to install neo-c. I can't support on alpine linux on arm and x86_64, I don't know why it can't be worked.
0.9.2.8 heap checker is allways enabled. It's guaranteed the heap memories are alive.
0.9.2.7 heap checker is allways enabled. It's guaranteed the heap memories are alive.
0.9.2.6 opt is ownd and heap only. DSL pipe filter with zero cost like rust.
0.9.2.5 I'm adding to zero cost iterator like rust.
0.9.2.4 refference count cycle checker is refined.
0.9.2.3 refference count cycle checker is refined.
0.9.2.2 parametor label checker.
0.9.2.1 ref, apan, opt are ok.
0.9.2.0 A bug exits in right value object automatically removed. Fixed it.
0.9.1.9 heap alive checker. stupied algorithm was removed. I learn about the knowledge of the algrorithm from gemini. span, opt, ref are maybe ok.
0.9.1.8 heap checker. changing ref, span, opt. Maybe OK.
0.9.1.7 changing ref, span, opt. Maybe OK.
0.9.1.6 changing ref, span, opt. Maybe OK.
0.9.1.6 char*/string file read/write helpers removed. Use xfopen(... )!, FILE*::fread, and FILE*::fwrite(... )! instead. trueby removed.
0.9.1.5 changing ref, span, opt.
0.9.1.4 My main server is in Mac.
0.9.1.3 Mac nad linux portabilities in C source level.
0.9.1.2 Mac nad linux portabilities in C source level.
0.9.1.1 INTEL OK
0.9.1.0 INTEL OK
0.9.0.9 Mac OS test and all OK.
0.9.0.8 ISO C11 compatibilities 90%. Cycle owned heap output warning. If you remove the waring you use _weak attribute.
0.9.0.7 more compatibilities C11. opt, ref, span is implemented and test.
0.9.0.6 testing opt, ref, span. Some content it's working.
0.9.0.5 testing opt, ref. Some content it's working.
0.9.0.4 & operand bug fixed.
0.9.0.3 span bug fixed. vector bug fixed.
0.9.0.2 rawptr implemented. vector implemeted.
0.9.0.1 defer implemeted.
0.9.0.0 smart pointer removed. slice removed. ref nad optional and span is maybe stable. null checker is enable when -cg option, by the default it's disable.
0.8.9.9 ref and optional autounwrap. force to unwrap with !
0.8.9.8 span more powerfull.
0.8.9.7 span, ref, optional bug fixed. maybe bug has remained.
0.8.9.6 null checker returns.
0.8.9.5 span, ref, optional coming.
0.8.9.4 array and pointer bug fixed by codex. no warning occur in tests
0.8.9.3 char*::read() method returned buffer*% at the time. Removed in 0.9.1.6.
0.8.9.2 optional and ref may be working. heap bug fixed.
0.8.9.1 span and ref bug fixed.
0.8.9.0 span implemeted at some content.
0.8.8.8 ref and optional bug fixed
0.8.8.7 ref and optional implemented. removed rawptr, removed storing field, array, loading field, array checker integrate.
0.8.8.6 storing field, array, loading field, array checker integrate.
0.8.8.5 tuple2<T,bool> can be used like Result<T>. and catch method implemented. case method bug is fixed.
0.8.8.4 slice, rawptr implemented.
0.8.8.3 smart_pointer implemented.
0.8.8.2 require borrow to asign heap object to none heap variable.
0.8.8.1 __attribute__ supported comonly.
0.8.8.0 tuple expression is now t(1,2,3). More compatibilities C lang.
0.8.7.9 Fixed bug.
0.8.7.8 maybe c++ is supported.  -cpp option. No type checking and generate code.
0.8.7.7 cpp treats module. In parser module is deleted. here documet is working.
0.8.7.6 channel variable attribute character changed from @ to |
0.8.7.5 compare method automatically defined. You can use sort method easilly.
0.8.7.4 Type checking.
0.8.7.3 Array with heap object is freed from this version. 
0.8.7.2 Remove here document. ccpp.c can't treat here document properly. In C level it's not required function, I think.
0.8.7.1 You can get if result value.
0.8.7.0 stackframe recording is allways enable. and optimize recording stackframe sysytem.
0.8.6.2 mf has serious bug of deleting file. it occurs deleting all user files. Refine neo-c and get rid of the bug.
0.8.6.1 Refine neo-c
0.8.6.0 Fine working generics and method generics. Test and fix some bugs.
0.8.5.9 Change here document spec and it's usefull.
0.8.5.8 Refine the source.
0.8.5.7 inline asm bug is fiexed.
0.8.5.6 Tested arm, x86_64 many linux distribution.
0.8.5.5 More refine Makefile to parallel compile.
0.8.5.4 Refine Makefile to parallel compile.
0.8.5.3 no trim the last line of Here Document.
0.8.5.2 tested on PICO
0.8.5.1 Fixed the bug when class not found.
0.8.5.0 The extension of neo-c is changed from .c to .nc. And compiler drivers uses shell script. and you can write # at head of line in here document.
0.8.3.0 tested on PICO
0.8.2.5 CPP has been wrritten by codex. And more compatibilities C lang. Almost C header can be work.
0.8.2.1 Refactoring
0.8.2.0 remove GC heap area. This system use calloc and free only. So you can debug with valgrind.
0.8.1.6 GC algorithm bug has been fixed. Some code can't be work maybe, but more properly heap algorith has come.
0.8.1.5 Transpiletime Reflection
0.8.1.4 Transpiletime Reflection
0.8.1.3 More readable output c source. And encount a bug fixed.
0.8.1.2 no type check field, variable, and function. More optimized the output C source
0.8.1.1 clang like option implemented.
0.8.1 Object initializer and number object initializer
0.8.0 Tested on x86 Fedora. There are some bugs. Fixed it.
0.7.5 Tested on some linux distributeion. all worked.
0.7.2 Linux(amd64, x86_64) tested.
0.7.1 Linux tested. Atomic supported. -std=c11 supported
0.7.0 more c compatiblities, array, pointer.
0.6.0 more c compatiblities, array, pointer.
0.5.5 more c compatiblities
0.5.2 more c compatiblities
0.5.1 more c compatiblities
0.5.0 Reborn
```

# Sample

```
#include <neo-c.h>

int main(int argc, char** argv)
{
    int a[5] = { 3, 2, 1, 2, 5 };
    
    var li = new list<int>.initialize_with_values(5, a);
    
    var li2 = li.sort_with_lambda(int lambda(int left, int right) {
        if(left == right) {
            return 0;
        }
        else if(left < right) {
            return -1;
        }
        else {
            return 1;
        }
    });
    
    var li3 = li.filter { it < 3 }.sort().map { it.to_string() + "0" }.join("+").split("\\\+");
    
    li3.to_string().puts();
    
    "A,B,C".split(",").join("\n").puts();
    
    "A,B,C,D,E".split(",").add(s"F").join("+").to_string().puts();
    
    [1,2,3,4,5].add(6).add(3).add(2).add(10).add(9).sort().map { it.to_string() }.join("\n").to_string().puts();
    
    "ABCDEFGJHI".scan(".").map { it * 5 }.join("\n").puts();
    
    return 0;
}
```

# Language specifications

The syntax is almost the same as C language. If you do not #include <neo-c.h>, you can use it as a normal C compiler.The generated C source depends only on the standard C library, so it should work on any system that can run C.

文法はC言語とほとんど一緒です。#include <neo-c.h>をしないと普通のCコンパイラとして使えます。出力するCソースは標準Cライブラリにしか依存していないため、Cが動く環境でしたら、動くはずです。

# HELLO WORLD

```
> vim a.nc
#include <stdio.h>

int main()
{
    puts("HELLO WORLD");
    return 0;
}
> neo-c a.nc
> ./a
HELLO WORLD
```

# grammar

It is compatible with the C language. I think most C header files can be included as is.

C言語と互換性があります。大抵のCヘッダーファイルはそのままincludeできると思います。

# Libraries

The grammar library includes list, map, tuple, buffer, and string. 

ライブラリにはlist, map, tuple, buffer, stringがあります。

# list

```
#incldue <neo-c.h>

int main(int argc, char** argv)
{
    list<char*>*% li = ["AAA", "BBB", "CCC", "DDD", "EEE"];
    
    foreach(it, li.sublist(0,3)) {   // "AAA"\n"BBB"\n"CCC"
        puts(it);
    }
    
    li.add("FFF").add("GGG");
    
    foreach(it, li) {   // "AAA"\n"BBB"\n"CCC"\n"DDD"\n"EEE"\n"FFF"\n"GGG"\n
        puts(it);
    }
    
    return 0;
}
```

li.each { puts(it); } can also access all list elements, but break, continue, and return cannot be used. It is more efficient to use foreach. foreach takes the first argument as the name of the variable containing the element, and the second argument as the list.

foreach is implemented as a macro.

li.each { puts(it); }としても全てのリストの要素にアクセスできますが、break, continu, returnが使えません。foreachを使った方が効率的にもいいです。foreachは第一引数が要素が入った変数の名前、第二引数がリストとなります。

Below is an explanation of all methods.

以下は全メソッドの解説です。

```C
list<T>*% initialize(list<T>*% self);
```

A constructor. It will be called by typing new list<int>();. Internally this is new list<int>.initialize();
It is an abbreviation of

The constructor must add a % to the first argument and add 1 to the reference count. If you do not add %, it will be determined that the heap can be freed and will be automatically freed. The return value must also be marked with a % to prevent it from being released.

Write return self; at the end of the constructor. It cannot be omitted.

You can easily write constructors by using the class function described later.

コンストラクタです。new list<int>();とすれば呼び出されます。これは内部的にはnew list<int>.initialize();
の略です。

コンストラクタは第一引数に%をつけてリファレンスカウントを＋１しなければなりません。%をつけてないと解放していいヒープだと判断されて自動的にfreeされます。戻り値も%をつけて解放されないようにしないといけません。

コンストラクタの最後にはreturn self;と書いてください。略することはできません。

後述するclass機能を使えば、簡単にコンストラクタなどが記述できます。

```C
var li = new list<int>.initialize();
li.add(1).add(2).add(3);

var li2 = new list<int>();
li2.add(1).add(2).add(3);
```

```C
list<T>*% initialize_with_values(list<T>*% self, int num_value, T^* values) 
```

Creates a list initialized with an array.

配列で初期化されたリストを作成します。

```C
int values[3] = { 1, 2, 3 };

var li = new list<int>.initialize_with_values(3, values);
```

```C
void finalize(list<T>* self) 
```

It's a destructor. Called when automatically deleted or manually deleted.

デストラクターです。自動的に消去される場合、手動でdeleteする場合に呼ばれます。

```C
list<int>* li = borrow new list<int>();
li.add(1).add(2).add(3);
delete li;  // finalize is called. finalizeが呼ばれる
```

borrow removes the heap from being automatically released and allows it to be assigned to a variable without a %. In this case the heap is managed manually and memory leaks will occur if you don't delete it. If a memory leak occurs When you run the program, the number of memory leaks will be displayed.  the stack frame at the location of the source file where the heap was generated will be displayed. I think it's easy to debug.

borrowはヒープの自動解放対象から外し、%をつけない変数に代入できるようにします。この場合ヒープは手動で管理されて、deleteしないとメモリリークが発生します。 メモリリークが発生した場合プログラムを実行するとメモリリークの回数が表示されます。もしメモリリークがあるとソースファイルの位置のスタックフレームが表示されます。デバッグも容易だと思います。

```C
list<T>*% clone(list<T>* self)
```

```C
var li = [1,2,3,4,5];
var li2 = clone li;   // cloneがよばれる
```

clone is a deep copy. Elements are also recursively cloned to duplicate memory. Since li and li2 have a reference count of 1, they will be automatically deleted when they exit the scope.

If you don't want to deep copy, follow the steps below.

cloneはディープコピーです。要素も再起的にcloneされて、メモリが複製されます。liとli2はリファレンスカウントが1のためスコープを抜けると自動的に消去されます。

ディープコピーしたくない場合は以下です。

```C
var li = [1,2,3,4,5];
var li2 = li;
```

li2 refers to the same thing as li. The heap of [1,2,3,4,5] has a reference count of 2, so when li and li2 exit the scope, the reference count is -2 and becomes 0, and [1,2,3,4,5 ] will be released.

li2はliと同じものを指しています。[1,2,3,4,5]のヒープはリファレンスカウントが２のためliとli2がスコープを抜けるとその時リファレンスカウントが-2されて0になり、[1,2,3,4,5]は解放されます。

```C
var li = [1,2,3,4,5];
list<int>* li2 = borrow li;
```

In this case, li and li2 refer to the same thing, but if li2 is accessed after li is released, a segmentation fault will occur.

In most cases, you can just add a % to the pointer. Reference count GC handles this well.

From 0.8.8.2 require borrow to asign heap object to none heap variable.

この場合もliとli2は同じものをさしていますが、liが解放された後にli2にアクセスするとセグメンテーションフォルトを起します。

たいていの場合はポインタに%をつけておけば大丈夫です。リファレンスカウントGCがうまく対処してくれます。

version 0.8.8.2からnone heapのものをheapの変数に代入するにはborrowが必要になりました。

```C
list<T>* add(list<T>* self, T item)
```

```C
var li = [s"ABC", s"DEF", s"GHQ"]:
li.add(s"OPQ");
```

s"ABC" is a string type character string and is allocated on the heap. The same is true for string("ABC"). string is defined as typedef char*% string; and can also be treated as a simple char type array. So, putting(s"ABC"); will output ABC\n. In this case, the generated string will be automatically freed.

li is list<string>*%. The reference count of s"OPQ" in li.add(s"OPQ"); is incremented by 1 and it is correctly stored in li. The stored s"ABC", s"DEF", s"GHQ", and s"OPQ" are correctly freed when li is freed.

s"ABC"はstring型の文字列でヒープに確保されます。string("ABC")としても同じです。stringはtypedef char*% string;と定義されていて、単なるchar型の配列としても扱えます。なのでputs(s"ABC");とするとABC\nが出力されます。この場合生成された文字列は自動的にfreeされます。

li is list<string>*%. The reference count of s"OPQ" in li.add(s"OPQ"); is incremented by 1 and it is correctly stored in li. The stored s"ABC", s"DEF", s"GHQ", and s"OPQ" are correctly freed when li is freed.

liはlist<string>*%です。li.add(s"OPQ");のs"OPQ"はリファレンスカウントが+1されて、正しくliに格納されます。liが解放されるとき格納された、s"ABC", s"DEF", s"GHQ", s"OPQ"は正しくfreeされます。

```C
T pop_front(list<T>* self) 
```

Delete the beginning of the list. If the heap is stored, the element will be freed. If it is a non-heap pointer such as "ABC", it will not be freed.

リストの先頭を削除します。ヒープが格納されていた場合要素がfreeされます。ヒープでなく"ABC"などヒープでないポインタの場合はfreeされません。

```C
list<T>* push_back(list<T>* self, T item)
```

Same as add.

addと同じです。

```C
string to_string(list<T>* self)
```

```C
    var li = ["ABC", "DEF", "GHQ"];
    puts(li.to_string());
```

[ABC,DEF,GHQ] will be output. to_string is executed on all elements. 

li is list<char*>*%, which stores string pointers. The stored element will not be freed because it is not char*%.

[ABC,DEF,GHQ]が出力されます。要素の全てにto_stringが実行されます。

liはlist<char*>*%で文字列のポインタが格納されています。char*%ではないため格納された要素はfreeされません。

```C
T^ begin(list<T>* self) 
T^ next(list<T>* self) 
bool end(list<T>* self) 
```

Defined for foreach. Use this if you want to access all elements.

foreachのため定義されてます。すべての要素にアクセスしたい場合使います。

```C
list<T>* each(list<T>* self, void* parent, void (*block)(void*, T^,int,bool*)) 
```

```C
    var li = ["ABC", "DEF", "GHI"];
    
    li.each {
        puts(it);
    }
```

ABC\nDEF\nGHI\n will be output. Method block arguments are stored in it, it2, and it3. In this case, it contains each element. Unlike foreach, break, continue, and return cannot be executed. Returning just escapes from the method block.

ABC\nDEF\nGHI\nが出力されます。メソッドブロックはit,it2,it3にメソッドブロックの引数が格納されます。この場合itは各要素が入っています。foreachと違い、break, continue, returnは実行できません。returnするとメソッドブロックから脱出するだけです。

```C
T item(list<T>* self, int position, T default_value) 
```

```C
    var li = ["ABC", "DEF", "GHI"];
    puts(li.item(0, null)); // ABC
    puts(li.item(-1, null)); // GHI
    puts(li.item(-9999, "")); // ""
```

default_value is the value returned in case of out-of-range access. If <0, the elements counted from the back are returned.

default_valueは範囲外アクセスの場合その値が返されます。<0の場合は後方から数えた要素が返されます。

```C
int length(list<T>* self)
```

```C
    var li = [1,2,3];
    puts(li.length().to_string()); // 3
```

Returns the number of elements.

要素の数が返されます。

```C
list<T>* insert(list<T>* self, int position, T item)
```

```C
    var li = [1,2,3];
    
    li.insert(1@position, 5@item); // [1,5,2,3]
```

Add element to position. @postion is an annotation and is treated as a comment.

li should be [1,5,2,3].

要素をpositionに追加します。@postionはアノテーションでコメントとして扱われます。

liは[1,5,2,3]となるはずです。

```C
list<T>* reset(list<T>* self) 
```

```C
    var li = [1,2,3];
    
    li.reset();
    
    li.length().to_string().puts(); // 0
```

Clears the element. 0 will be output.

要素をクリアします。0が出力されます。

```C
list<T>* remove(list<T>* self, T^ item, bool by_pointer=false);
```

```C
    var li = [1,2,3];
    
    li.remove(3); // [1,2]
    
    li.to_string().puts();
```

Delete items that match item and equals.
li is [1,2].

itemとequalsがマッチするものを削除します。
liは[1,2]です。

If you add by_pointer:true to the prametor, remove the address matching object.

by_pointer:trueとするとポインタのアドレスが同じものをremoveします。



```C
list<T>* delete(list<T>* self, int head, int tail)
```

```C
    var li = [1,2,3,4,5];
    
    li.delete(3,-1); // [1,2,3]
    li.delete(0,1); // [2,3];
```

Delete what is in the range. -1 is the tail.
li is [1,2,3].

範囲に入っているものを削除します。-1は末尾です。
liは[2,3]です。

```C
list<T>* replace(list<T>* self, int position, T item)
```

```C
    var li = [1,2,3,4,5];
    
    li.replace(1, 7); // [1,7,3,4,5]
```

Replace the item.
li is [1,7,3,4,5]. If the element is a heap, the reference count of the replaced element will be -1, and if the reference count is 0, it will be deleted.

アイテムを置き換えます。
liは[1,7,3,4,5]です。要素がヒープの場合置き換える要素はリファレンスカウントが-1されて、リファレンスカウントが0なら削除されます。

```C
int find(list<T>* self, T^ item, int default_value, bool by_pointer=false)
```

```C
    var li = [1,2,3,4,5];
    
    li.find(3, -1@default_value); // 2
```

Returns the position from the beginning of the element matched by equals. In this case it is 2. default_value is the value if not found.

equalsがマッチする要素の先頭からの位置を返します。この場合2です。default_valueは見つからなかった場合の値です。

```C
vector<T>*% to_vector(list<T>* self)
```

```C
    var li = [1,2,3];
    var v = li.to_vector();
    v.to_string().puts(); // v[1,2,3]
```

Convert list to vector.

listをvectorに変換します。

```C
bool equals(list<T>* left, list<T>* right)
```

```C
    [1,2,3].equals([1,2,3]).to_string().puts(); // true
```

Checks whether the object has the same argument and content. Equals is executed for each element and returns true if it is true for all elements.

オブジェクトが引数と内容が一緒か確認します。要素ごとにequalsが実行されすべての要素で真ならtrueを返します。

```C
list<T>*% sublist(list<T>* self, int begin, int tail) 
```

```C
    [1,2,3,4,5].sublist(0,2); // [1,2]
    [1,2,3,4,5].sublist(3,-1); // [4,5]
    [1,2,3,4,5].sublist(3,-2); // [4]
```


```C
T operator_load_element(list<T>* self, int position) 
```

```C
    var li = [1,2,3,4,5];
    
    printf("%d\n", li[3]); // 4
    printf("%d\n", li[-1]); // 5
    printf("%d\n", li[-9999]); // 0
```

If the index is not found, 0 clear value returned.

もしインデックスが見つからないなら0clearされた値が返されます。

```C
void operator_store_element(list<T>* self, int position, T item) 
```

```C
    var li = [1,2,3,4,5];
    
    li[0] = 123; // [123,2,3,4,5]
```

```C
list<T>*% operator_load_range_element(list<T>* self, int begin, int tail) 
```

```C
    var li = [1,2,3,4,5];
    
    li[0..2].to_string().puts(); // [1,2]
    li[3..-1].to_string().puts(); // [4,5]
```

```C
bool operator_equals(list<T>* self, list<T>* right) 
```

```C
    [1,2,3] === [1,2,3]; // true
    [1,2,2] === [1,2,3]; // false
```

equals is called for each element.

各要素にequalsが呼ばれます。

```C
bool operator_not_equals(list<T>* left, list<T>* right) 
```

```C
    [1,2,3] !== [1,2,3]; // false
    [1,2,2] !== [1,2,3]; // true
```

```C
bool contained(list<T>* self, T^ item, bool by_pointer=false);
```

```C
    [1,2,3].contained(3); // true
    [1,2,3].contained(4); // false
```

```C
list<T>*% merge_list_with_lambda(list<T>* left, list<T>* right, int (*compare)(T^,T^)) 
list<T>*% merge_sort_with_lambda(list<T>* self, int (*compare)(T^,T^)) 
list<T>*% sort_with_lambda(list<T>* self, int (*compare)(T^,T^)) 
```

```C
    [3,7,2,5].sort_with_lambda(int lambda(int left, int right) {
        if(left < right) {
            return -1;
        }
        else if(left > right) {
            return 1;
        }
        else {
            return 0;
        }
        
        return 0;
    }); // [2,3,5,7]
```

Sort by lambda expression.

lambda式でソートします。

```C
list<T>*% merge_list(list<T>* left, list<T>* right) 
list<T>*% merge_sort(list<T>* self) 
list<T>*% sort(list<T>* self) 
```

```C
    [3,7,2,5].sort(); // [2,3,5,7]
```

ソートにはcompareメソッドが必要です。もし、ない場合は自動的に生成されます。第一フィールドのcompareが呼ばれます。

Sorting requires a compare method. If one is not present, it is generated automatically. The compare of the first
  field is used.

```C
template<R> list<R>*% map(list<T>* self, void* parent, R (*block)(void*, T^))
```

```C
    ["1","2","3"].map { atoi(it) }  // [1,2,3]
```

Executes an expression on each element and returns a list of results. 

各要素に式を実行して、その結果のリストを返します。

```C
list<T>*% reverse(list<T>* self) 
```

```C
    [1,2,3].reverse(); // [3,2,1]
```

```C
list<T>*% uniq(list<T>* self, bool by_pointer=false);
```

```C
    [8,8,2,2,3,3].uniq(); // [8,2,3]
```

Delete adjacent identical elements. It may not work unless you use sort().

隣あった同じ要素を削除します。sort()しないとダメかもしれません。

```C
list<T>*% filter(list<T>* self, void* parent, bool (*block)(void*, T^))
```

```C
    [1,2,3,4,5].filter { it > 2 };  // [3,4,5]
```

Rust-like terminal helpers are also available as normal collection methods.

Rust風の終端ヘルパーも通常のコレクションメソッドとして使えます。

```C
    var li = [1,2,3,4,5].filter { it % 2 == 1 };

    li.count();                          // 3
    li.len();                            // 3
    li.is_empty();                       // false
    li.any { it == 3 };                 // true
    li.all { it < 6 };                  // true
    li.position(-1) { it == 3 };        // 1
    li.find_value(-1) { it > 3 };       // 5
    li.nth(-1, -1);                     // 5
    li.last(-1);                        // 5
    li.sum();                           // 9
    li.product();                       // 15
    li.min(-1);                         // 1
    li.max(-1);                         // 5
```

Available methods are `count`, `len`, `is_empty`, `any`, `all`, `position`, `find_value`, `nth`, `last`, `sum`, `product`, `min`, and `max`.

使えるメソッドは `count`, `len`, `is_empty`, `any`, `all`, `position`, `find_value`, `nth`, `last`, `sum`, `product`, `min`, `max` です。

For empty collections, `count()` is `0`, `any()` is `false`, `all()` is `true`, `sum()` is zero-initialized, `product()` is `1`, and the others return `default_value` when they have one.

空コレクションでは `count()` は `0`、`any()` は `false`、`all()` は `true`、`sum()` はゼロ初期化値、`product()` は `1` で、それ以外は `default_value` を受け取るものはそれを返します。

```C
list<T>*% operator_add(list<T>*% left, list<T>*% right) 
```

```C
    [1,2,3] + [4,5]; // [1,2,3,4,5]
```

```C
list<T>*% operator_mult(list<T>* left, int right) 
```

```C
    [1,2,3] * 2; // [1,2,3,1,2,3]
```

```C
string join(list<T>* self, char* sep=" ") 
```

```C
    [1,2,3].join("+");    // 1+2+3
```

# vector

```
#include <neo-c.h>

int main(int argc, char** argv)
{
    int values[5] = { 1, 2, 3, 4, 5 };
    
    vector<int>*% v = new vector<int>.initialize_with_values(5, values);
    
    foreach(it, v[1..4]) {   // 2\n3\n4\n
        printf("%d\n", it);
    }
    
    v.add(6).add(7);
    
    foreach(it, v) {   // 1\n2\n3\n4\n5\n6\n7\n
        printf("%d\n", it);
    }
    
    return 0;
}
```

vector can also be iterated with foreach. Internally it uses begin, next, and end.

vectorもforeachで反復できます。内部的にはbegin, next, endが使われます。

vector can be written with `v[]`. `[]` is a list, and `v[]` is a vector.

vectorは`v[]`で書けます。`[]`はlistで、`v[]`はvectorです。

```C
var v = v[1,2,3];
v.to_string().puts();   // v[1,2,3]
```

Below is an explanation of all methods.

以下は全メソッドの解説です。

```C
vector<T>*% initialize(vector<T>*% self);
```

A constructor. It will be called by typing new vector<int>();. Internally this is new vector<int>.initialize();

コンストラクタです。new vector<int>();とすれば呼び出されます。これは内部的にはnew vector<int>.initialize();の略です。

```C
var v = new vector<int>.initialize();
v.add(1).add(2).add(3);

var v2 = new vector<int>();
v2.add(1).add(2).add(3);
```

```C
vector<T>*% initialize_with_values(vector<T>*% self, int num_value, T^* values)
```

Creates a vector initialized with an array.

配列で初期化されたvectorを作成します。

```C
int values[3] = { 1, 2, 3 };

var v = new vector<int>.initialize_with_values(3, values);
var v2 = v[1,2,3];
```

```C
void finalize(vector<T>* self)
```

It's a destructor. Called when automatically deleted or manually deleted.

デストラクターです。自動的に消去される場合、手動でdeleteする場合に呼ばれます。

```C
vector<int>* v = borrow new vector<int>();
v.add(1).add(2).add(3);
delete v;  // finalize is called. finalizeが呼ばれる
```

```C
vector<T>*% clone(vector<T>* self)
```

```C
var v = new vector<int>.initialize();
v.add(1).add(2).add(3);

var v2 = clone v;
```

clone is a deep copy. If T is a heap type, each element is cloned.

cloneはディープコピーです。Tがヒープ型の場合、各要素もcloneされます。

```C
vector<T>* add(vector<T>* self, T item)
```

Appends an element to the end. If the internal buffer is full, it is expanded automatically.

末尾に要素を追加します。内部バッファが足りない場合は自動的に拡張されます。

```C
var v = new vector<int>();
v.add(1).add(2).add(3);
```

```C
T item(vector<T>* self, int index, T default_value)
```

```C
int values[3] = { 10, 20, 30 };
var v = new vector<int>.initialize_with_values(3, values);

printf("%d\n", v.item(0, 0));   // 10
printf("%d\n", v.item(-1, 0));  // 30
printf("%d\n", v.item(99, -1)); // -1
```

default_value is returned for out-of-range access. Negative indexes count from the end.

default_valueは範囲外アクセスの場合に返されます。負のインデックスは末尾から数えます。

```C
bool equals(vector<T>* left, vector<T>* right)
```

```C
var v = new vector<int>();
v.add(1).add(2).add(3);

var v2 = new vector<int>();
v2.add(1).add(2).add(3);

v.equals(v2).to_string().puts();   // true
```

Checks whether the size and contents are equal. equals is executed for each element.

サイズと内容が等しいか確認します。各要素にequalsが実行されます。

```C
void replace(vector<T>* self, int index, T value)
```

```C
int values[3] = { 1, 2, 3 };
var v = new vector<int>.initialize_with_values(3, values);

v.replace(1, 7);   // v[1,7,3]
v.replace(-1, 9);  // v[1,7,9]
```

Replaces the element at index. Negative indexes are supported.

indexの要素を置き換えます。負のインデックスも使えます。

```C
int find(vector<T>* self, T^ item, int default_value)
```

```C
int values[5] = { 1, 2, 3, 4, 5 };
var v = new vector<int>.initialize_with_values(5, values);

printf("%d\n", v.find(3, -1));   // 2
printf("%d\n", v.find(99, -1));  // -1
```

Returns the first matching index. default_value is returned if not found.

最初に一致した位置を返します。見つからない場合はdefault_valueが返ります。

vector also supports the same terminal helpers as list: `count`, `len`, `is_empty`, `any`, `all`, `position`, `find_value`, `nth`, `last`, `sum`, `product`, `min`, `max`.

vector でも list と同じ終端ヘルパー `count`, `len`, `is_empty`, `any`, `all`, `position`, `find_value`, `nth`, `last`, `sum`, `product`, `min`, `max` が使えます。

```C
int values[4] = { 1, 2, 3, 4 };
var v = new vector<int>.initialize_with_values(4, values);

printf("%d\n", v.count());                    // 4
printf("%d\n", v.len());                      // 4
printf("%d\n", v.is_empty());                 // 0
printf("%d\n", v.position(-1) { it == 3 });  // 2
printf("%d\n", v.last(-1));                  // 4
printf("%d\n", v.sum());                     // 10
```

```C
int length(vector<T>* self)
```

```C
int values[3] = { 1, 2, 3 };
var v = new vector<int>.initialize_with_values(3, values);

printf("%d\n", v.length());   // 3
```

Returns the number of elements.

要素数を返します。

```C
int alloc_size(vector<T>* self)
```

```C
int values[3] = { 1, 2, 3 };
var v = new vector<int>.initialize_with_values(3, values);

printf("%d\n", v.alloc_size());
```

Returns the number of bytes currently used for stored elements. This is sizeof(T) * length().

現在格納されている要素に使われているバイト数を返します。sizeof(T) * length()です。

```C
void reset(vector<T>* self)
```

```C
int values[3] = { 1, 2, 3 };
var v = new vector<int>.initialize_with_values(3, values);

v.reset();
printf("%d\n", v.length());   // 0
```

Clears all elements and reinitializes the internal storage.

全要素をクリアし、内部ストレージを初期化し直します。

```C
T^ begin(vector<T>* self)
T^ next(vector<T>* self)
bool end(vector<T>* self)
```

Defined for foreach. Use this if you want to access all elements manually.

foreachのため定義されています。手動で全要素にアクセスしたい場合に使います。

```C
T operator_load_element(vector<T>* self, int position)
```

```C
int values[5] = { 1, 2, 3, 4, 5 };
var v = new vector<int>.initialize_with_values(5, values);

printf("%d\n", v[0]);      // 1
printf("%d\n", v[-1]);     // 5
printf("%d\n", v[9999]);   // 0
```

If the index is not found, a zero-cleared value is returned.

インデックスが見つからない場合は0クリアされた値が返されます。

```C
void operator_store_element(vector<T>* self, int index, T item)
```

```C
int values[3] = { 1, 2, 3 };
var v = new vector<int>.initialize_with_values(3, values);

v[0] = 123;   // v[123,2,3]
```

```C
vector<T>*% operator_load_range_element(vector<T>* self, int begin, int tail)
```

```C
int values[5] = { 1, 2, 3, 4, 5 };
var v = new vector<int>.initialize_with_values(5, values);

v[0..2].to_string().puts();   // v[1,2]
v[3..-1].to_string().puts();  // v[4,5]
```

Creates a new vector from the specified range. tail is exclusive. Negative indexes are supported.

指定範囲から新しいvectorを作成します。tailは含まれません。負のインデックスも使えます。

```C
bool operator_equals(vector<T>* left, vector<T>* right)
```

```C
int a[3] = { 1, 2, 3 };
int b[3] = { 1, 2, 3 };
int c[3] = { 1, 2, 4 };

var v1 = new vector<int>.initialize_with_values(3, a);
var v2 = new vector<int>.initialize_with_values(3, b);
var v3 = new vector<int>.initialize_with_values(3, c);

printf("%s\n", (v1 === v2).to_string());   // true
printf("%s\n", (v1 === v3).to_string());   // false
```

```C
bool operator_not_equals(vector<T>* left, vector<T>* right)
```

```C
int a[3] = { 1, 2, 3 };
int b[3] = { 1, 2, 4 };

var v1 = new vector<int>.initialize_with_values(3, a);
var v2 = new vector<int>.initialize_with_values(3, b);

printf("%s\n", (v1 !== v2).to_string());   // true
```

```C
vector<T>*% operator_add(vector<T>* left, vector<T>* right)
```

```C
int a[2] = { 1, 2 };
int b[2] = { 3, 4 };

var v1 = new vector<int>.initialize_with_values(2, a);
var v2 = new vector<int>.initialize_with_values(2, b);

(v1 + v2).to_string().puts();   // v[1,2,3,4]
```

Concatenates two vectors and returns a new vector.

2つのvectorを連結した新しいvectorを返します。

```C
vector<T>*% operator_mult(vector<T>* left, int n)
```

```C
int a[2] = { 1, 2 };
var v = new vector<int>.initialize_with_values(2, a);

(v * 3).to_string().puts();   // v[1,2,1,2,1,2]
```

Repeats the vector n times and returns a new vector.

vectorをn回繰り返した新しいvectorを返します。

```C
vector<T>* sort_with_lambda(vector<T>* self, int (*compare)(T^, T^))
```

```C
int values[4] = { 3, 7, 2, 5 };
var v = new vector<int>.initialize_with_values(4, values);

v.sort_with_lambda(int lambda(int left, int right) {
    if(left < right) {
        return -1;
    }
    else if(left > right) {
        return 1;
    }
    else {
        return 0;
    }
});
```

Sorts the vector in place using the given compare function.

指定したcompare関数でその場ソートします。

```C
vector<T>* sort(vector<T>* self)
```

```C
int values[4] = { 3, 7, 2, 5 };
var v = new vector<int>.initialize_with_values(4, values);

v.sort().to_string().puts();   // v[2,3,5,7]
```

Sorts the vector in place using compare.

compareを使ってその場ソートします。

```C
string to_string(vector<T>* self)
```

```C
int values[3] = { 1, 2, 3 };
var v = new vector<int>.initialize_with_values(3, values);

v.to_string().puts();   // v[1,2,3]
```

Returns a string representation. vector is displayed with the prefix v.

文字列表現を返します。vectorはvを付けて表示されます。

```C
span<T*>*% to_span(vector<T>* self)
```

```C
int values[3] = { 1, 2, 3 };
var v = new vector<int>.initialize_with_values(3, values);
var s = v.to_span();
printf("%d\n", s[1]);   // 2
```

Creates a span that references the internal buffer of the vector.

vectorの内部バッファを参照するspanを作成します。

# map

map is a dictionary.

mapは辞書です。

```
#include <neo-c.h>

int main(int argc, char** argv)
{
    map<char*, int>*% ma = new map<char*, int>();
    
    ma.insert("AAA", 1);
    ma.insert("BBB", 2);
    ma.insert("CCC", 3);
    
    foreach(it, ma) {
        int item = ma[it];
        
        printf("element %s %d\n", it, item);
    }
    
    return 0;
}
```

```C
map<T,T2>*% initialize(map<T,T2>*% self)
```

```C
var ma = new map<char*,int>();
ma.insert("AAA", 1);
ma.insert("BBB", 2);
ma.insert("CCC", 3).insert("DDD", 4);
```

```C
map<T,T2>*% initialize_with_values(map<T,T2>*% self, int num_keys, T^* keys, T2^* values) 
```

```
    char* keys[] = { "AAA", "BBB", "CCC", "DDD" };
    int values[] = { 1, 2, 3, 4 };
    
    var ma = new map<char*,int>.initialize_with_values(4, keys, values);
```

```C
void finalize(map<T,T2>* self)
```

```C
map<T, T2>*% clone(map<T, T2>* self)
```

```
    var ma = ["AAA":1, "BBB":2, "CCC":3];
    var ma2 = clone ma;
```


```C
string to_string(map<T,T2>* self)
```

```
    ["AAA":1, "BBB":2, "CCC":3].to_string().puts();   // [AAA:1,BBB:2,CCC:3]
```

All elements and keys must implement to_string(). All basic types of neo-c have to_string() implemented. Struct will automatically define to_string method.

すべての要素とキーにto_string()が実装されている必要があります。neo-cの基本的な型はすべてto_string()が実装されてます。to_stringは構造体の場合自動的に定義されます。

```C
T2 at(map<T, T2>* self, T^ key, T2 default_value, bool by_pointer=false);
```

```
    ["AAA":1, "BBB":2, "CCC":3].at("AAA", -1).to_string().puts();  // 1
```

Takes value by key. This is the value if default_value is not found.

キーで値をとります。default_valueが見つからない場合の値です。

```C
map<T,T2>* remove(map<T, T2>* self, T key, bool by_pointer=false);
```

```
    var ma = ["AAA":1, "BBB":2, "CCC":3].remove("AAA");
    ma.to_string().puts();   // [BBB:2, CCC:3]
```

Delete value by key.

キーで値を削除します。

```C
T^ begin(map<T, T2>* self)
T^ next(map<T, T2>* self) 
bool end(map<T, T2>* self) 
```

It is for foreach. To access all keys:

foreachのためにあります。すべてのキーにアクセスするには以下のようにします。

```
    var ma = ["AAA":1, "BBB":2, "CCC":3];
    
    foreach(key, ma) {
        var item = ma[key];
        
        printf("%s %d\n", key, item);
    }
```

The output is AAA 1\nBBB 2\n CCC 3\n.

map also supports the same terminal helpers, but they operate on keys.

map でも同じ終端ヘルパーが使えますが、対象は値ではなくキーです。

```C
var ma = [3:30, 1:10, 4:40];

printf("%d\n", ma.count());                    // 3
printf("%d\n", ma.len());                      // 3
printf("%d\n", ma.is_empty());                 // 0
printf("%d\n", ma.position(-1) { it == 1 });  // 1
printf("%d\n", ma.last(-1));                  // 4
printf("%d\n", ma.sum());                     // 8
```

出力はAAA 1\nBBB 2\n CCC 3\nです。

```C
void rehash(map<T,T2>* self) 
```

For internal use.

内部的に使用します。

```C
map<T,T2>* insert(map<T,T2>* self, T key, T2 item, bool by_pointer=false);
map<T,T2>* put(map<T,T2>* self, T key, T2 item, bool by_pointer=false);
```

```
    var ma = ["AAA":1].insert("BBB",2).insert("CCC",3);
    ma.to_string().puts(); // [AAA:1,BBB:2,CCC:3]
```

```C
T2 operator_load_element(map<T, T2>* self, T^ key) 
```

```
    var ma = ["AAA":1,"BBB":2,"CCC":3];
    var item = ma["AAA"];
    item.to_string().puts(); // 1
```

If the key is not found, zero clear value is returned.

キーが見つからない場合0クリアされた値が返されます。

```C
void operator_store_element(map<T, T2>* self, T key, T2 item) 
```

```
    var ma = ["AAA":1, "BBB":2];
    ma["CCC"] = 3;
    ma.to_string().puts(); // [AAA:1,BBB:2,CCC:3]
```

```C
bool equals(map<T, T2>* left, map<T, T2>* right)
```

```
    ["AAA":1,"BBB":2,"CCC":3].equals(["AAA":1,"BBB":2,"CCC":3]); // true
    ["AAA":1].equals(["BBB":2]); // false
```

```C
bool operator_equals(map<T, T2>* left, map<T,T2>* right) 
```

```
    ["AAA":1,"BBB":2,"CCC":3] === ["AAA":1,"BBB":2,"CCC":3]; // true
    ["AAA":1] === ["BBB":2]; // false
```

```C
bool operator_not_equals(map<T, T2>* left, map<T,T2>* right) 
```

```
    ["AAA":1,"BBB":2,"CCC":3] !== ["AAA":1,"BBB":2,"CCC":3]; // false
    ["AAA":1] !== ["BBB":2]; // true
```

```C
bool find(map<T, T2>* self, T^ key, bool by_pointer=false);
```

```
    ["AAA":1, "BBB":2].find("AAA"); // true;
    ["AAA":1, "BBB":2].find("CCC"); // false;
```

Returns true if the key is included.

キーが含まれればtrueを返します。

```C
map<T,T2>*% operator_add(map<T,T2>* left, map<T,T2>* right) 
```

```
    (["AAA":1] + ["BBB":2]).to_string().puts(); // [AAA:1,BBB:2]
```

```C
map<T,T2>*% operator_mult(map<T,T2>* left, int right) 
```

```
    (["AAA":1] * 2).to_string().puts(); // [AAA:1,AAA:1]
```

```C
list<T>*% keys(map<T, T2>* self)
```

```
    ["AAA":1, "BBB":2, "CCC":3].keys().to_string().puts();  // [AAA,BBB,CCC]
```

```C
list<T2>*% values(map<T, T2>* self) 
```

```
    ["AAA":1, "BBB":2, "CCC":3].values().to_string().puts();  // [1,2,3]
```

```C
vector<T>*% to_vector(map<T, T>* self)
```

```C
    var ma = [1:10, 2:20, 3:30];
    ma.to_vector().to_string().puts(); // v[1,10,2,20,3,30]
```

Flatten map entries into vector as `key, item, key, item, ...`.

mapの要素を`key, item, key, item, ...`の順でvectorに展開します。
keyとitemの型は同じである必要があります。

# tuple

A tuple is a collection of elements of different types. It may be called a simple structure.

タプルは型の違う要素を持つコレクションです。簡易的な構造体と呼べるかもしれません。

```
#include <neo-c.h>

int main(int argc, char** argv)
{
    tuple3<int, int, char*>*% tu = new tuple3<int, int, char*>(1, 2, "ABC");
    
    printf("%d %d %s\n", tu.v1, tu.v2, tu.v3);
    
    return 0;
}
```

Up to 5 tuples are defined.

tupleは5つまで定義されてます。

```
    tuple5<int,int,int,string,char*>*% tu = t(1,2,3,s"ABC","DEF");
    
    tu.to_string().puts(); // (1,2,3,ABC,DEF)
```

int, bool have the same meaning as tuple2<int, bool>*%. Used to return multiple values. When you want to store multiple values in each variable, you can use var a,b = div(1,1); Int is placed in a and bool is placed in b.

To access the element, use v1 etc.

int, boolは`tuple2<int, bool>*%`と同じ意味です。複数の値を返すために使います。複数の値を各変数に格納したい時、個の場合はvar a,b = div(1,1);とすればいいです。aにint, bにboolが入ります。

要素にアクセスするにはv1などとします。

```
    var tu = t(1,2,"ABC");
    
    tu.v1 === 1; // true
    tu.v2 === 2; // true
    tu.v3 === "ABC"; // true
```

`tup: int, string`は`tuple2<int,string>*%`と同じです。`list<tuple2<int,string>*%>*%`の代わりに`list<int, string>*%`は使えませんが、`list<tup:int,string>*%`は使えます。

`tup: int, string` is the same as `tuple2<int,string>*%`. You cannot use `list<int, string>*%` instead of `list<tuple2<int,string>*%>*%`, ​​but you can use `list<tup:int,string>*%`.

Tuples also have fixed-size helpers.

tuple にも固定長ヘルパーがあります。

```
    t(1, 2, 3).len();     // 3
    t(1, 2, 3).count();   // 3
```

# buffer

Buffer is memory that can be appended. Expression of buffer is b"".

bufferは追記できるメモリーです。

bufferの値の表現はb""です。

```
#include <neo-c.h>

int main(int argc, char** argv)
{
    buffer*% buf = new buffer();
    
    buf.append_str("ABC");
    buf.append_str("DEF");
    
    puts(buf.to_string()));
    
    buffer*% buf2 = new buffer();
    
    buf2.append_int(1);
    buf2.append_int(2);
    buf2.append_int(3);
    
    int* p = buf2.head_pointer();
    
    printf("%d\n", *p);  // 1
    p++;
    printf("%d\n", *p);  // 2
    p++;
    printf("%d\n", *p);  // 3
    
    return 0;
}
```

```C
buffer*% buffer*::initialize(buffer*% self);
```

```
    var buf = new buffer();
```

```C
void buffer*::finalize(buffer* self);

buffer*% buffer*::clone(buffer* self);
```

```
    var buf = new buffer();
    buf.append_int(1);
    buf.append_int(2);
    buf.append_int(3);
    
    var buf2 = clone buf;
```

```C
int buffer*::length(buffer* self);
```

```
    var buf = new buffer();
    buf.append_char('a');
    buf.append_char('b');
    buf.append_char('c');
    
    buf.length().to_string().puts(); // 3
```

Returns the number of bytes of memory.

メモリーのバイト数を返します。

```C
void buffer*::reset(buffer* self);
```

```
    var buf = new buffer();
    
    buf.append_char('a');
    buf.append_char('b');
    buf.append_char('c');
    
    buf.reset();
    
    buf.length().to_string().puts(); // 0
```

Clear memory.

メモリーをクリアします。

```C
void buffer*::trim(buffer* self, int len);
```

Delete trailing memory by len.

lenだけ末尾のメモリを削除します。

```
    var buf = new buffer();
    
    buf.append_str("ABCDEFG");
    buf.trim(3);
    buf.to_string().puts(); // ABCD
```

```C
buffer* buffer*::append(buffer* self, char* mem, size_t size);
```

Add memory by mem size.

memのsizeだけメモリを追加します。

```
    var buf = new buffer();
    buf.append("ABCDEFG", 2);
    
    buf.to_string().puts(); // AB
```

```C
buffer* buffer*::append_char(buffer* self, char c);
buffer* buffer*::append_str(buffer* self, char* str);
buffer* buffer*::append_nullterminated_str(buffer* self, char* str);
buffer* buffer*::append_int(buffer* self, int value);
buffer* buffer*::append_long(buffer* self, long value);
buffer* buffer*::append_short(buffer* self, short value);
```

Add memory.

メモリーを追加します。

```C
buffer* buffer*::alignment(buffer* self);
```

Align memory.

メモリーのアライメントを取ります。

```C
int buffer*::compare(buffer* left, buffer* right);
```

Compare the buffer sizes. <0 means smaller on the left, >0 means smaller on the right. == 0 and have the same size.
Used in sort.

bufferの大きさを比べます。<0で左が小さい、>0で右が小さい。== 0で同じ大きさです。
sortで使います。

```C
buffer*% string::to_buffer(char* self);
buffer*% char*::to_buffer(char* self);
```

Convert string to buffer.

文字列をbufferに変換します。

```
    var buf = "ABCDEFG".to_buffer();
    buf.append_str("HIJ");
    
    buf.to_string().puts(); // ABCDEFGHIJ
```

```C
string buffer*::to_string(buffer* self);
```

Convert buffer to string.

bufferを文字列に変換します。

```C
static inline buffer*% char[]::to_buffer(char* self, size_t len) ;
static inline buffer*% short[]::to_buffer(short* self, size_t len) ;
static inline buffer*% int[]::to_buffer(int* self, size_t len) ;
static inline buffer*% long[]::to_buffer(long* self, size_t len) ;
static inline buffer*% float[]::to_buffer(float* self, size_t len) ;
static inline buffer*% double[]::to_buffer(double* self, size_t len) ;
```

```C
    char a[4] = { 'A', 'B', 'C', '\0' };
    
    var buf = a.to_buffer(4);
    
    puts(buf.to_string()); // ABC
```

```C
static inline list<char>*% char[]::to_list(char* self, size_t len) ;
static inline list<short>*% short[]::to_list(short* self, size_t len) ;
static inline list<int>*% int[]::to_list(int* self, size_t len) ;
static inline list<long>*% long[]::to_list(long* self, size_t len) ;
static inline list<float>*% float[]::to_list(float* self, size_t len) ;
static inline list<double>*% double[]::to_list(double* self, size_t len) ;
```

```C
    int a[3] = { 3, 2, 1 };
    
    a.to_list(3).sort().each {
        printf("%d\n", it);
    }
```

# string 

```
#include <neo-c.h>

int main(int argc, char** argv)
{
    puts(xsprintf("%d", 1 + 1));     // ==> 2
    puts(string("ABC").substring(0, 1));  // ==> "A"
    
    return 0;
}
```

```C
int string::length(char* str);
```

```
    s"ABC".length(); // 3
```

```C
int char*::length(char* str);
```

```
    "ABC".length(); // 3
```

```C
string char*::substring(char* str, int head, int tail);
string string::substring(char* str, int head, int tail);
```

```
    "ABC".substring(0,2); // AB
```

```C
string string::operator_load_range_element(char* str, int head, int tail);
string char*::operator_load_range_element(char* str, int head, int tail);
```

```
    "ABC"[0..2]; // AB
```

```C
string char*::reverse(char* str) ;
string string::reverse(char* str) ;
```

```
    "ABC".reverse();   // CBA
```

```C
string xsprintf(char* msg, ...);
```

```
    var str = xsprintf("%d+%d+%d", 1,2,3); // 1+2+3
```

```C
static inline string string::xsprintf(char* self, char* msg, ...)
static inline string char*::xsprintf(char* self, char* msg, ...)
```

```
    s"ABC".xsprintf("[%s]").puts(); // [ABC]
```

```C
static inline string int::xsprintf(int self, char* msg, ...)
```

```
    [1,2,3,4,5].item(0, -1).xsprintf("[%d]\n").puts();  // [1]
```

```C
string char*::delete(char* str, int head, int tail) ;
string string::delete(char* str, int head, int tail);
```

```
    var str = s"ABCDEFG".delete(0,1); // CDEFG
```
```C
list<string>*% string::split_char(char* self, char c) ;
list<string>*% char*::split_char(char* self, char c);
```

```C
    s"A,B,C".split_char(','); // [A,B,C]
```

path related wrapper

pathの関連のラッパー

```C
string xrealpath(char* path);
```

string version of realpath(3)

realpath(3)のstring版

```C
xrealpath("/aaa/../bbb"); // /bbb
```

Maybe.

多分。

```C
string xbasename(char* path);
```

string version of basename(3)

basename(3)のstring版

```C
string xextname(char* path);
```

return extension

拡張子を返す

```C
string xdirname(char* path);
```

return directory

ディレクトリを返す

```C
string xnoextname(char* path);
```

Returns the file name without the extension.

拡張子をとったファイル名を返す。

```C
Result<int>*% FILE*::fwrite(FILE* f, const char* str);
```

```
    FILE* f = fopen("AAA", "a");
    
    f.fwrite("ABC")!;
    
    f.fclose()!;
```

`FILE*::write` was renamed to `FILE*::fwrite` and now returns `Result<int>*%`.
Use `!` when you want to panic on error.

`FILE*::write`は`FILE*::fwrite`に改名され、`Result<int>*%`を返すようになりました。
エラー時にpanicさせたい場合は`!`を使います。

```C
Result<buffer*%>*% FILE*::fread(FILE* f);
```

```
    FILE* f = fopen("AAA", "r");

    buffer*% buf = f.fread()!;
    string data = buf.to_string();

    f.fclose()!;
```

`FILE*::read` was renamed to `FILE*::fread` and now returns `Result<buffer*%>*%`.

`FILE*::read`は`FILE*::fread`に改名され、`Result<buffer*%>*%`を返すようになりました。

```C
Result<int>*% FILE*::fclose(FILE* f);
```

similar. Use `!` when you want to panic on close error.

同様。closeエラー時にpanicさせたい場合は`!`を使います。

```C
Result<FILE*>*% FILE*::fprintf(FILE* f, const char* msg, ...);
```

```
    FILE* f = fopen("AAA", "a");
    
    f.fprintf("%d\n", 1+1)!;
    
    f.fclose()!;
```

similar. Use `!` when you want to panic on error.
同様。エラー時にpanicさせたい場合は`!`を使います。

```C
Result<list<string>*%>*% FILE*::readlines(FILE* f);
```

```
    FILE* out = xfopen("FILE", "w")!;
    out.fwrite("AAA\nBBB\nCCC\n")!;
    out.fclose()!;
    
    FILE* f = xfopen("FILE", "r")!;
    
    var li = f.readlines()!;
    
    li[0].puts(); // AAA
    li[1].puts(); // BBB
    li[2].puts(); // CCC
```

`char*::read`, `string::read`, `char*::write`, and `string::write` were removed because file IO on plain strings can hide unintended bugs.
Open files explicitly with `xfopen(... )!`, then use `FILE*::fread` or `FILE*::fwrite(... )!`.

`char*::read`、`string::read`、`char*::write`、`string::write`は削除されました。
普通の文字列にファイルIOを生やすと意図しないバグを隠しやすいためです。
`xfopen(... )!`で明示的にファイルを開き、`FILE*::fread`または`FILE*::fwrite(... )!`を使ってください。

```
buffer*% buf = xfopen("01main.nc", "r")!.fread()!;
```

```
xfopen("OUTPUT","w")!.fwrite("AAA\n")!.fclose()!
```

エラー時はpanicします。

# Default parameters, parameter labels

``` C
#include<stdio.h>

int fun(int x = 123, int y = 345, int z = 456) 
{
    printf("x %d y %d z %d\n", x, y, z);
}

struct sData 
{
    int x;
    int y;
    int z;
};

int sData*::fun(sData* self, int x = 123, int y = 345, int z = 456)
{
    self.x = x;
    self.y = y;
    self.z = z;
}

void sData*::show(sData* self)
{
    printf("x %d y %d z %d\n", self.x, self.y, self.z);
}

int main(int argc, char** argv) 
{
    fun();           // x 123 y 345 z 456 are outputed
    fun(y:2);        // x 123 y 2 z 456
    
    fun(y:1, x:3);   // x 3 y 1 z 456
    
    fun(1);          // x 1 y 345 z 456
    fun(1,2);        // x 1 y 2 z 456
    
    sData data;
    (&data).fun(1,2,3);
    (&data).show();   // x 123 y 345 z 456
    
    (&data).fun(y:2); // x 123 y 2 z 456
    (&data).show();   // x 123 y 2 z 456
    
    (&data).fun(1);
    (&data).show();   // x 1 y 345 z 456
    
    return 0;
}
```

# operator overloads

``` C
string char*::operator_mult(char* str, int n);
string string::operator_mult(char* str, int n);
bool char*::operator_equals(char* left, char* right);
bool string::operator_equals(char* left, char* right);

    "ABC" * 3  // => "ABCABCABC"
    [1,2] * 3  // => [1,2,1,2,1,2]
    "ABC" === "ABC" // => true
    "ABC !== "DEF" // => true
    [1,2] + [3] // => [1,2,3]
    
    auto ma1 = ["AAA":1, "BBB":2]
    
    xassert("map test", ma1["AAA"] == 1)
    ma1["CCC"] = 3
```

``` C
+ operator_add
- operator_sub
* operator_mult
/ operator_div
% operator_mod
=== operator_equals
!== operator_not_equals
> operator_gt
>= operator_gteq
< operator_le
<= operator_leeq
[x] = y operator_store_element
[x]  operator_load_element
! operator_logical_denial
<< operator_left_shift
>> operator_right_shift
& operator_and
^ operator_xor
| operator_or
~ operator_commplement
```

Runtime `/` and `%` evaluate the right operand once, check it for zero, and panic with `exit(2)` before performing the operation when the divisor is zero.

実行時の `/` と `%` は右辺を一度だけ評価し、0なら演算前にpanicして `exit(2)` で終了します。

# mixin-layers system

``` C
#include <stdio.h>

int fun(char* str) version 1
{
    puts(str);
    return 1;
}

int fun(char* str) version 2
{
    int n = inherit(str);

    return n + 1;
}

int fun(char* str) version 3
{
    int n = inherit(str);

    return n + 1;
}

int main()
{
    if(fun("HELLO MIXIN-LAYERS") == 3) {
        puts("OK");
    }

    return 0;
}
```

# Annotation

``` C
int fun(bool flag) 
{
    if(flag) {
        puts("TRUE");
    }
    else {
        puts("FALSE");
    }
}

int main()
{
    fun(false@flag);

    return 0;
}
```

@[a-zA-Z][a-zA-Z_0-9]* is a comment of expression.

# multiple assign

``` C
#include <neo-c.h>

int, string fun(int n, string m) 
{
    return t(n, m);
}

int main(int argc, char** argv)
{
    var n, m = fun(1, string("AAA"));
    
    printf("n %d m %s\n", n, m);
    
    return 0;
}
```

# Interface

``` C
#include <neo-c.h>

interface sBase
{
    void show();
};

struct sChildA
{
    int x;
    int y;
    int z;
    string str;
}

sChildA*% sChildA*::initialiize(sChildA*% self)
{
    self.x = 1;
    self.y = 2;
    self.z = 3;
    self.str = string("ABC");
    
    return self;
}

void sChildA*::show(sChildA* self)
{
    printf("x %d y %d z %d str %s\n", self.x, self.y, self.z, self.str);
}

struct sChildB
{
    int X;
    int Y;
    int Z;
};

sChildB*% sChildB*::initialize(sChildB*% self)
{
    self.X = 111;
    self.Y = 222;
    self.Z = 333;
    
    return self;
}

void sChildB*::show(sChildB* self)
{
    printf("X %d Y %d Z %d\n", self.X, self.Y, self.Z);
}

int main(int argc, char** argv)
{
    list<sBase*%>*% li = new list<sBase*%>();
    
    li.add(new sChildA() implements sBase);
    li.add(new sChildB() implements sBase);
    
    foreach(it, v) {
        it.show();
    }
    
    return 0;
}

```

```
#include <neo-c.h>

class sData
{
    int a;
    int b;
    
    void show() {
        puts(s"a \{self.a} b \{self.b}");
    }
}

class sData2 extends sData
{
    int c;
    
    void show() {
        puts(s"a \{self.a} b \{self.b} c \{self.c}");
    }
}

interface IShow
{
    void show();
}

int main(int argc, char** argv) 
{
    var data2 = new sData2 { a:111, b:222, c:333 };
    
    sData*% data = data2;
    
    puts(s"\{data2}");
    
    puts("interface");
    
    IShow*% idata[2];
    idata[0] = data implements IShow;
    idata[1] = data2 implements IShow;
    
    for(int i=0; i<2; i++) {
        idata[i].show();
    }
                                    
    return 0;
}
```


# Using C


# Emmbeded expression string

```
int main(int argc, char* argv)
{
    string a = s"abc";
    
    puts(a);          // abc
    
    string b = S"def";
    
    puts(b);          // def
    
    int c = 123;
    
    puts(s"c == \{c}");   // c == 123;
    
    puts(s"1 + 1 == \{1+1}");   // 1+1 == 2;
    
    return 0;
}
```

# System call errro handling like perl

```C
#include <neo-c.h>

int main(int argc, char** argv)
{
    FILE* f = fopen("NOTHING", "r") or die("file not found");
    
    return 0;
}
```
    
and or or is like perl

# method block

```
#include <neo-c.h>

int main(int argc, char** argv)
{
    3.times {
        printf("%d\n", it);
    }
    
    return 0;
}
```

```
#include <neo-c.h>

int main(int argc, char** argv)
{
    ["1", "2", "3"].map { atoi(it) }.filter { it > 1 }.each { it. printf("%d\n"); }
    
    return 0;
}
```

# String libraries

SEE neo-c.h. The regex engine is implemented using the standard C libraries. 

If you only want string, buffer, list, and regex helpers, you can include `neo-c-str.h`.

文字列、buffer、list、regex helperだけ使いたいなら`neo-c-str.h`だけincludeできます。

```C
#include <neo-c-str.h>
```

`neo-c-str.h` can be used without `neo-c.h`.
It provides `string`, `buffer`, `list<T>`, `xsprintf()`, substring/delete/reverse helpers, and regex-based string methods.

`neo-c-str.h`は`neo-c.h`なしでも使えます。
`string`, `buffer`, `list<T>`, `xsprintf()`, substring/delete/reverse系, regexベースの文字列メソッドを持っています。

sample

``` C
#include <neo-c.h>

int main()
{
    xassert("char_match test", "ABC".match("A"));
    xassert("char_index test", "ABC".index("B", -1) == 1);
    xassert("char_rindex test", "ABCABC".rindex("B", -1) == 4);
    xassert("char_index_regex", "ABC".index_regex("B", -1) == 1);
    xassert("char_rindex_regex", "ABCABC".rindex_regex("B", -1) == 4);

    string str = string("ABC");

    str.replace(1, 'C');

    xassert("char_replace", str === "ACC"))
    xassert("char_multiply", "ABC".multiply(2) === "ABCABC");

    xassert("char_sub", "ABC".sub("B", "C" === "ACC");

    auto li = "ABC".scan(".");

    xassert("char_scan", li[0] === "A" && li[1] === "B" && li[2] === "C");

    auto li2 = "A,B,C".split(",");

    xassert("char_split", li2[0] === "A" && li2[1] === "B" && li2[2] == "C");

    auto li3 = "A,B,C".split_char(',');

    xassert("char_split_char", li3[0] === "A" && li3[1] === "B" && li3[2] === "C");
    
    auto li4 = "A,,B,,C".split_str(",,");

    xassert("char_split_str", li4[0] === "A" && li4[1] === "B" && li4[2] === "C");
    
    xassert("char_delete", "ABC".delete(0,1) === "BC);

    xassert("wchar_substring", wstring("ABC").substring(0,1) === wstring("A"));
    
    auto li6 = "A,B,C".split_str(",");
    
    xassert("join", li6.join(" ") === "A B C");
    
    auto li7 = "A,B,C".split(",");
    
    xassert("split test", li7[0] === "A" && li6[1] === "B" && li7[2] === "C");
    xassert("rindex_regex test", "ABCABC".rindex_regex("CBA", -1) == 5);
    xassert("sub_block test", "ABCABCABC".sub_block("ABC") { string("X"); } === "XXX");
    xassert("scan_block test", "123 456 789".scan_block("[0-9][0-9][0-9]") { it.substring(0, 1); }.join("") === "147");
    
    auto li8 = "ABC".scan(".");
    
    xassert("scan test", li8[0] === "A" && li8[1] === "B" && li8[2] === "C");
    
    var bufX = "ABC".to_buffer();
    bufX.append_str("DEF");
    
    xassert("to_buffer test", bufX.to_string() === "ABCDEF");
    xassert("split block test", "ABC,DEF,GHI".split_block(",") { it.substring(0,1); }.join("") === "ADG");
    xassert("regex test", "ABC".scan(".").join("") === "ABC");
    
    xassert("regex equals test", "aaa" === "aaa");
    
    return 0;
}
```

## neo-c-str.h manual

`string`

`string` is `typedef char*% string;`.
It is a heap-managed string type.

`string`は`typedef char*% string;`です。
ヒープ管理される文字列型です。

```C
#include <neo-c-str.h>

int main()
{
    string a = xsprintf("%s %d", "value", 10);
    puts(a);
    
    return 0;
}
```

Core string helpers

```C
int char*::length(const char* str);
string char*::substring(const char* str, int head, int tail);
string char*::delete(char* str, int head, int tail);
string char*::reverse(const char* str);
string char*::printable(char* str);
string char*::sub_plain(char* self, char* str, char* replace);
```

`substring(head, tail)` supports negative indexes.
`delete(head, tail)` removes a range.
`printable()` escapes control characters for debugging.
`sub_plain()` is plain string replacement without regex.

`substring(head, tail)`は負のindexを使えます。
`delete(head, tail)`は範囲削除です。
`printable()`はデバッグ用に制御文字を見える形にします。
`sub_plain()`はregexを使わない通常の置換です。

```C
#include <neo-c-str.h>

int main()
{
    puts("ABCDE".substring(1, 4));   // BCD
    puts("ABCDE".substring(-3, -1)); // CD
    puts("ABCDE".delete(1, 3));      // ADE
    puts("ABC".reverse());           // CBA
    puts("A\tB\n".printable());      // A\tB\n
    puts("abcabc".sub_plain("ab", "XY")); // XYcXYc
    
    return 0;
}
```

Regex string helpers

```C
bool char*::match(char* self, const char* reg, bool ignore_case=false);
int char*::index_regex(const char* self, const char* reg, int default_value=-1, bool ignore_case=false);
int char*::rindex_regex(const char* self, const char* reg, int default_value=-1, bool ignore_case=false);
list<string>*% char*::scan(const char* self, const char* reg, bool ignore_case=false);
list<string>*% char*::split(const char* self, const char* reg, bool ignore_case=false);
list<string>*% char*::split_str(const char* self, const char* str);
list<string>*% char*::split_char(char* self, char c);
string char*::sub(char* self, const char* reg, const char* replace, bool global=true, bool ignore_case=false);
string char*::sub_block(char* self, const char* reg, bool global=true, bool ignore_case=false, void* parent, string (*block)(void* parent, char* match_string, list<string>* group_strings));
list<string>*% char*::scan_block(const char* self, const char* reg, bool ignore_case=false, void* parent, string (*block)(void* parent, char* match_string, list<string>* group_strings));
```

`scan()` returns all matches.
`split()` uses regex separators.
`split_str()` and `split_char()` use plain separators.
`sub()` uses regex replacement.
`sub_block()` and `scan_block()` let you transform each match.

`scan()`は全matchを返します。
`split()`はregex区切りです。
`split_str()`と`split_char()`は通常の区切りです。
`sub()`はregex置換です。
`sub_block()`と`scan_block()`は各matchごとに変換できます。

```C
#include <neo-c-str.h>

int main()
{
    xassert("match", "Hello123".match("[0-9]+"));
    xassert("index_regex", "abc123xyz".index_regex("[0-9]+", -1) == 3);
    xassert("rindex_regex", "ab12cd34".rindex_regex("[0-9]+", -1) == 6);
    
    var a = "A,B,C".split_str(",");
    var b = "A B\tC".split("[ \t]+");
    var c = "ABCABC".scan("ABC");
    
    xassert("split_str", a.length() == 3 && a[0] === "A" && a[2] === "C");
    xassert("split regex", b.length() == 3 && b[1] === "B");
    xassert("scan", c.length() == 2 && c[0] === "ABC");
    
    puts("abc123".sub("[0-9]+", "X")); // abcX
    puts("123 456 789".scan_block("[0-9]+") { it.substring(0, 1) }.join("")); // 147
    puts("abc123def".sub_block("[0-9]+") { s"[" + it + s"]" }); // abc[123]def
    
    return 0;
}
```

`buffer`

```C
buffer*% buffer*::initialize(buffer*% self);
buffer* buffer*::append(buffer* self, const char* mem, size_t size);
buffer* buffer*::append_str(buffer* self, const char* mem);
buffer* buffer*::append_char(buffer* self, char c);
buffer* buffer*::append_format(buffer* self, const char* msg, ...);
void buffer*::reset(buffer* self);
int buffer*::length(buffer* self);
string buffer*::to_string(buffer* self);
```

Use `buffer` when you want efficient incremental string building.

文字列を段階的に組み立てたいときは`buffer`を使います。

```C
#include <neo-c-str.h>

int main()
{
    var buf = new buffer();
    
    buf.append_str("neo");
    buf.append_char('-');
    buf.append_format("%s %d", "c", 11);
    
    xassert("buffer", buf.to_string() === "neo-c 11");
    
    buf.reset();
    buf.append_str("reset ok");
    puts(buf.to_string());
    
    return 0;
}
```

`list<T>`

`neo-c-str.h` also contains `list<T>`.
So you can use regex/string APIs without `neo-c.h` and still get `list<string>*%` results directly.

`neo-c-str.h`には`list<T>`も入っています。
そのため`neo-c.h`なしでもregex/string APIの戻り値である`list<string>*%`をそのまま使えます。

```C
#include <neo-c-str.h>

int main()
{
    var li = "A,B,C".split_str(",");
    
    foreach(it, li) {
        puts(it);
    }
    
    return 0;
}
```

`wstring`

Wide string helpers are also available.

```C
wstring wchar_t*::substring(const wchar_t* str, int head, int tail);
wstring wchar_t*::delete(wchar_t* str, int head, int tail);
wstring wchar_t*::reverse(const wchar_t* str);
wstring wchar_t*::printable(const wchar_t* str);
string wchar_t*::to_string(const wchar_t* wstr);
```

Use this when you want wide-character string slicing and conversion.

ワイド文字列の切り出しや変換に使えます。

```C
#include <neo-c-str.h>

int main()
{
    wstring ws = wstring("ABC");
    xassert("wstring", ws.substring(0, 2) === wstring("AB"));
    puts(ws.to_string());
    
    return 0;
}
```

# stackfame

```C
> vin a.nc
#include <neo-c.h>

int fun()
{
    stackframe();
    return 0;
}

int fun2()
{
    return fun();
}

int main(int argc, char** argv)
{
    fun2();
    
    return 0;
}
> neo-c a.nc
> ./a
stackframe
fun
fun2
main
```

stackframe always enabled from version 0.8.7.0.

On Linux and macOS normal libc builds, panic paths use the native
`execinfo` backtrace first. Build with `-g` to keep debug information and
emit `#line` mappings, so panic output can show `.nc` source file names and
line numbers. Without `-g`, or after stripping, the output falls back to
function names and addresses. `-bare`, `-micro`, and other freestanding
targets keep the lightweight neo-c stackframe chain and do not depend on
`execinfo`.

# Template

```C
template<R> R fun(R x, R y)
{
    return x + y;
}

int main(int argc, char** argv)
{
    fun(1,2).printf("%d\n");
    
    return 0;
}
```

type inference is works.

```C
#include <neo-c.h>

int main(int argc, char** argv)
{
    [1,2,3].map { it.to_string() }.each { puts(it); }
    
    return 0;
}
```

```
#include <neo-c.h>

struct proc_alias_target {
    char*% name;
};

template<T> T first_item(list<T>* xs, T^ default_value)
{
    return xs.nth(0, default_value);
}

int main()
{
    proc_alias_target*% p = new struct proc_alias_target;
    p.name = string("x");

    var xs = new list<proc_alias_target*%>.initialize();
    xs.add(p);

    proc_alias_target*% q = first_item(xs, null);

    return q.name === "x";
}
```

# Memory leak detector

Memory leak detection remains enabled by default. It records the allocation
count, object type, source file, and source line, then reports allocations
that remain alive when `main` exits. The allocation-time call stack consumes
additional memory for every live allocation, so it is now optional.

Use `-memleak-stacktrace` when the full allocation call stack is needed:

```sh
neo-c -memleak-stacktrace a.nc
```

通常でもメモリリーク検出は有効です。確保回数、型、ソースファイル、行番号を
記録し、`main`終了時に未解放オブジェクトを報告します。確保時の呼び出し履歴は
各オブジェクトの管理領域を大きくするため、必要な場合だけ
`-memleak-stacktrace`で有効にします。

`map`のハッシュテーブルは使用率50%で、従来の10倍拡張ではなく2倍拡張に
なりました。
`vector`は2倍ではなく1.5倍ずつ拡張します。再確保時の一時的なピークメモリと
未使用容量を抑えるための設定です。

``` 
#include <neo-c.h>

void fun()
{
    int* a = borrow new int;
    *a = 123;

    printf("%d\n", *a);
}
                            
int main(int argc, char** argv) 
{
    fun();
                                    
    return 0;
}
~/neo-c # neo-c a.nc
~/neo-c # ./a
123
#1 0x3a73b3b0 (int*) a.nc 5: come_alloc_mem_from_heap_pool, come_calloc, fun, main, 
1 memory leaks. 1 alloc, 0 free.
```

# Class

```C
#include <neo-c.h>

class sData
{
    int x;
    int y;
    
    new(int x, int y)
    {
        self.x = x;
        self.y = y;
    }
    
    void show()
    {
        printf("x %d y %d\n", self.x, self.y);
    }
};

int main(int argc, char** argv)
{
    sData*% data = new sData(111, 222);
    
    data.show();
    
    return 0;
}
```

```C
#include <neo-c.h>

class sData
{
    int x;
    int y;
    
    new(int x, int y)
    {
        self.x = x;
        self.y = y;
    }
    
    void show()
    {
        printf("x %d y %d\n", self.x, self.y);
    }
};

class sData2 extends sData
{
    int z;
   
    new(int x, int y, int z)
    {
        self.x = x;
        self.y = y;
        self.z = z;
    }
};

interface sDataRun
{
    void show();
};

int main(int argc, char** argv)
{
    sData*% data = new sData(111, 222);
    
    data.show();
    
    sData2*% data2 = new sData2(1, 2, 3);
    
    data2.show();
    
    sDataRun*% inf = new sData2(1,2,3) implements sDataRun;
    
    inf.show();
    
    return 0;
}

```

```C
#include <neo-c.h>

class sData
{
    int x;
    int y;
    
    new(int x, int y)
    {
        self.x = x;
        self.y = y;
    }
    
    void show()
    {
        printf("x %d y %d\n", self.x, self.y);
    }
};

class sData2 extends sData
{
    int z;
    
    new(int x, int y, int z)
    {
        self.super(x,y);
        self.z = z;
    }
    
    void show()
    {
        self.super();
        
        printf("z %d\n", self.z);
    }
};
```

```C
#include <neo-c.h>

class sData
{
    new(int x, int y)
    {
        int self.x = x;
        int self.y = y;
    }
    
    void show()
    {
        printf("x %d y %d\n", self.x, self.y);
    }
};

int main(int argc, char** argv)
{
    sData*% data = new sData(111, 222);
    
    data.show();
    
    return 0;
}
```

# Module

```
#include <neo-c.h>

#module MModule(T)
{
    T a;
    T b;
}

class sData
{
    MModule(int);
    
    new(int a, int b) {
        self.a = a;
        self.b = b;
    }
    
    void show() {
        printf("%d %d\n", self.a, self.b);
    }
}

int main(int argc, char** argv)
{
    var data = new sData(111, 222);
    
    data.show();
    
    return 0;
}
```

# uniq

```
> vin a.h
using c
{
#include <stdio.h>
}

extern int gGlobalVar;

uniq int gGlobalVar = 777;

uniq void fun()
{
    printf("%d\n", gGlobalVar);
}
> vin a.nc
#include "a.h"

int main(int argc, char** argv)
{
    fun();
    
    return 0;
}
> vin b.nc
#include "a.h"

void fun2()
{
    fun();
}
> neo-c -c b.nc
> neo-c a.nc b.o
> ./a
777
```

uniq function and global variable added to main module.
In other module, not defined contents.

transpile with -uniq option force to write function and global variable body to the source.

# Output of standard c targeted C Source

Don't include neo-c-net.h and neo-c-pthread.h. Inlucde neo-c.h only. It depends the standard c library only. 

# Extra libraries

Please use the C language extension library. The strength of neo-c is that the C library can be used as is.

拡張ライブラリはC言語のものを使ってください。neo-cの強みはCライブラリがそのまま使える点です。

# Net libraries

See neo-c-net.h

It's very slow the transpile.

For `neo-c-net.h`, use normal loops with `begin()`, `next()`, and `end()` like `webweb`.
The old backtick zero cost iterator style is no longer the recommended API here.

ネットワークAPIは`neo-c-net.h`をincludeして使います。
`webweb` と同じように `begin()`, `next()`, `end()` を使う普通のループで扱ってください。
backtick の zero cost iterator スタイルは、この用途では前提にしません。

```C
#include <neo-c-net.h>
```

Available APIs

利用できるAPI

```C
server_socket_iterator*% server_socket(int port=8080, int socket_family=AF_INET, int socket_type=SOCK_STREAM, int protocol=0, bool reuse=false);
client_socket_iterator*% client_socket(int port=8080, char* address="127.0.0.1");
Result<string>*% client_socket2(int port, const char* data, const char* address="127.0.0.1");
httpd_socket_iterator*% httpd_socket(int port=8080, int socket_family=AF_INET, int socket_type=SOCK_STREAM, int protocol=0, bool reuse=false);
httpsd_socket_iterator*% httpsd_socket(int port=443, bool reuse=false);
```

Use these APIs with explicit loops.

明示的なループで使います。

```C
var server = httpd_socket(port:8080, reuse:true);

for(var it = server.begin(); !server.end(); it = server.next()) {
    char buf[1024] = { 0 };
    int size = read(it, buf, 1023);
    
    if(size <= 0) {
        continue;
    }
    
    const char* response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "\r\n"
        "hello\n";
    
    write(it, response, strlen(response));
}
```

`webweb/src/main.nc` uses this same style for both HTTP and HTTPS servers.

`webweb/src/main.nc` でも HTTP/HTTPS の両方でこの形を使っています。

## neo-c-net.h manual

`socket_fd`

`socket_fd` is `typedef int socket_fd;`.
`socket_fd::write(string)` is a shortcut of `write(fd, str, str.length())` and returns `Result<int>*%`.

`socket_fd`は`typedef int socket_fd;`です。
`socket_fd::write(string)`は`write(fd, str, str.length())`の短縮で、`Result<int>*%`を返します。

`server_socket()`

This is for stateful TCP servers.
The first iteration accepts one connection.
After that, `next()` returns the same socket again unless you call `server.reconnect()`.
Use this when you want to keep one client session and explicitly accept the next client only after disconnect or protocol end.

状態を持つTCPサーバです。
最初の反復で1接続acceptします。
その後は`server.reconnect()`を呼ばない限り、`next()`でも同じsocketが返ります。
1クライアントを継続的に処理し、切断時だけ次のacceptに進みたいときに使います。

```C
#include <neo-c-net.h>

int main()
{
    var server = server_socket(port:3366, reuse:true);
    
    for(var it = server.begin(); !server.end(); it = server.next()) {
        char buf[1024] = { 0 };
        int size = read(it, buf, 1023);
        
        if(size <= 0) {
            server.reconnect();
            continue;
        }
        
        buf[size] = '\0';
        
        if(strcmp(buf, "exit") == 0) {
            break;
        }
        
        it.write(xsprintf("echo: %s\n", buf))!;
    }
    
    return 0;
}
```

`client_socket()`

This opens one TCP client connection.
`begin()` connects once and returns the socket.
`next()` returns the same socket, so `break` is usually useful.

1回TCP接続を開くクライアントです。
`begin()`で1回connectしてsocketを返します。
`next()`でも同じsocketが返るので、通常は`break`を使います。

```C
#include <neo-c-net.h>

int main()
{
    var client = client_socket(port:3366);

    for(var it = client.begin(); !client.end(); it = client.next()) {
        it.write(s"ping")!;
        
        char buf[1024] = { 0 };
        int size = read(it, buf, 1023);
        
        if(size > 0) {
            buf[size] = '\0';
            puts(buf);
        }

        break;
    }
    
    return 0;
}
```

`client_socket2()`

This is a simple helper.
It connects, writes all `data`, reads one response, closes the socket, and returns the response string as `Result<string>*%`.

簡易ヘルパです。
connectして、`data`を全部writeして、1回readして、socketを閉じて、その応答文字列を`Result<string>*%`で返します。

```C
string result = client_socket2(3366, "ping")!;
puts(result);
```

`httpd_socket()`

This is for simple HTTP servers.
Each iteration accepts a new client socket.
`next()` closes the current client and accepts the next one.

単純なHTTPサーバ向けです。
各反復ごとに新しいclient socketをacceptします。
`next()`では現在のclientを閉じて次をacceptします。

```C
#include <neo-c-net.h>

int main()
{
    var server = httpd_socket(port:8080, reuse:true);

    for(var it = server.begin(); !server.end(); it = server.next()) {
        char data[2048] = { 0 };
        int size = read(it, data, 2047);
        
        if(size <= 0) {
            continue;
        }
        
        const char* response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "\r\n"
            "OK\n";
        
        write(it, response, strlen(response));
    }
    
    return 0;
}
```

`httpsd_socket()`

This is the SSL version.
Each iteration accepts one SSL connection.
You need `cert.pem` and `key.pem` in the current directory.
Use `SSL_read()` and `SSL_write()` on `it`.

SSL版です。
各反復で1つのSSL接続をacceptします。
カレントディレクトリに`cert.pem`と`key.pem`が必要です。
`it`には`SSL_read()`と`SSL_write()`を使います。

```C
#include <neo-c-net.h>

int main()
{
    var server = httpsd_socket(port:443, reuse:true);

    for(var it = server.begin(); !server.end(); it = server.next()) {
        char data[2048] = { 0 };
        int size = SSL_read(it, data, 2047);
        
        if(size <= 0) {
            continue;
        }
        
        const char* response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "\r\n"
            "HTTPS OK\n";
        
        SSL_write(it, response, strlen(response));
    }
    
    return 0;
}
```

Use `break` when you want one-shot connect/accept behavior in these loops.

このループでは、1回だけconnect/acceptしたいなら`break`を使ってください。

# Omit return statment

```C
[1,2,3].map { return it.to_string(); }.each { puts(it); } 
    <=> [1,2,3].map { it.to_string() }.each { puts(it); ]
```

Omitting semicolon at the function block end means return statment.

# Pattern Matching

```C
#include <neo-c.h>

int main(int argc, char** argv)
{
    strcmp("A", "B").case {
        (Value < 0) { puts("Lesser");  }
        (Value == 0) { puts("Equal"); }
        (Value > 0) { puts("Greater"); }
    }
    strcmp("A", "A").case {
        (Value == 0) puts("Equal"); }
        else puts("Not Equal");
    }
    
    return 0;
}
```

```C
#include <neo-c.h>

char*% fun(int a)
{
    if(a == 0) {
        return null;
    }
    else {
        return xsprintf("%d", a);
    }
}

int main(int argc, char** argv)
{
    fun(0).elif {
        puts("null");
    }
    
    return 0;
}
```

```C
#include <neo-c.h>

char*% fun(int a)
{
    if(a == 0) {
        return null;
    }
    else {
        return xsprintf("%d", a);
    }
}

int main(int argc, char** argv)
{
    fun(1).if {
        puts(Value);
    }
    
    return 0;
}
```

```C
#include <neo-c.h>

int fun()
{
    return 0;
}

int fun2()
{
    return -1;
}

int main(int argc, char** argv)
{
    fun().less {
        puts("ERR");
        return 1;
    }
    fun2().less {
        puts("ERR");
        return 1;
    }
    
    return 0;
}
```

Pattern matching can have the result when the last line without semicolon

```
#include <neo-c.h>

int main(int argc, char** argv)
{
    char buf[16];
    
    FILE* f = fopen("NOTHING", "r").elif {
        fopen("c-src/01main.c", "r")
    }
    
    int n = fread(buf, 1, 16, f);
    
    buf[n] = '\0';
    
    puts(buf);
    
    f.fclose()!;
    
    return 0;
}
```

if statment is also can have the result when the last line without semicolon

```
#include <neo-c.h>

int main(int argc, char** argv)
{
    int n = if(1) { 777 };
    
    puts(s"\{n}");
    
    return 0;
}
```

# Reflection

```
defined(ID)
is_static(ID)
is_heap(ID)
is_const(ID)
is_uniq(ID)
is_type(ID)
type(ID)
param_types(ID, NUM)
num_param_types(ID)
num_fields(ID)
field_types(ID, NUM)
type(ID)
puts(STR)
is_inline(ID)
is_struct(ID)
is_union(ID)
is_enum(ID)
is_interface(ID)
is_generics(ID)
is_module(ID)
undef(ID)
```

```
#include <stdio.h>
#include <stdlib.h>

int fun()
{
    puts("aaa");
}

void fun2()
{
    puts("bbb");
}

static int fun4(int a, char* b)
{
    return 1;
}

static const int gUHO;

struct sData
{
    int a;
    char* b;
};

if(num_param_types(fun4) == 2) {
    if(param_types(fun4, 0) == int) {
        int a = 888;
    }
}
elif (num_fields(sData) == 2) {
    if(field_types(sData, 0) == int) {
        int a = 7;
    }
    else {
        int a = 999;
    }
}
elif (defined(fun2) && defined(fun)) {
    int a = 3;
}
else {
    int a = 2;
}

int main(void) {
    printf("a %d\n", a);
    return 0;
}
```

```
#include <stdio.h>

var a = "int a;"
var b = "int b;"
var c = "1"

if($c * 5 == 5){
    int c;
}
if($c . 1 == 11){
    int d;
}

eval "long \{$a . $b}"

int main(int argc, char** argv)
{
    printf("a %d b %d c %d d %d\n", a, b, c, d);
    
    return 0;
}
```

```
#include <stdio.h>

var a = "int a;"
var b = "int b;"
var c = "1"

struct sData
{
    int a;
    int b;
    int c;
};

if($c * 4 == 4){
    int c;
}
if($c . 1 == 11){
    int d;
}

static int** aaaa[128];

puts(is_static(type(aaaa)));

puts("HELLO REFLECTION");
puts(sizeof(sData));

int funX() { }
puts(result_type(funX));

eval "long \{$a . $b}"

int main(int argc, char** argv)
{
    printf("a %d b %d c %d d %d\n", a, b, c, d);
    printf("size %ld\n", sizeof(struct sData));
    
    return 0;
}
```

# Object initializer

```C
#include <neo-c.h>

struct sData
{
    int a;
    int b;
};

int main(int argc, char** argv)
{
    var data = new sData { a:123, b:234 };
    
    data.to_string().puts(); // new sData {a:123,b:234}
    
    return 0;
}
```

# Thread, Channel

```C
#include <neo-c.h>
#include <neo-c-pthread.h>

int fun(int a, int b)
{
    printf("%d %d\n", a, b);
    
    return 3;
}

int main(int argc, char** argv)
{
    int| a = __channel__;
    int| b = __channel__;
    
    var thread = come {
        fun(1, 2);
        a <- 111;
        a <- 222;
    }
    
    b <- 222;
    
    come_join(thread);
    
    come_poll {
        a {
            printf("a %d\n", <-a);
        }
        
        b {
            printf("b %d\n", <-b);
        }
    }
    
    return 0;
}
```

```
#include <neo-c.h>
#include <neo-c-pthread.h>

int main(int argc, char** argv)
{
    int| a = __channel__;
    int| b = __channel__;
    
    var thread = come {
        a <- 111;
        a <- 222;
    }
    var thread2 = come {
        b <- 333;
        b <- 444;
    }
    
    while(1) {
        come_poll {
            a {
                printf("a %d\n", <-a);
            }
            b {
                printf("b %d\n", <-b);
            }
            else {
                break;
            }
        }
    }
    
    come_join(thread);
    come_join(thread2);
    
    return 0;
}
```

スレッド間のデータのやり取りはグローバル変数やローカル変数で行わず全てチャネル経由で行ってください。
グローバル変数やローカル変数でやり取りするとデータの競合が起こります。

When exchanging data between threads, do not use global or local variables, but rather use channels.
Exchanging data using global or local variables will result in data conflicts.

use mutex to prevent from conflicts.

```
#include <neo-c.h>
#include <neo-c-pthread.h>

int main(int argc,char** argv)
{
    var li = new come_mutex<list<int>*%>([1,2,3]);
    
    var thread2 = come {
        li.sync() {
            it.to_string().puts();
        }
        sleep(1);
        li.sync() {
            it.to_string().puts();
        }
    }
    
    var thread = come {
        li.sync() {
            it.add(4);
        }
        li.sync() {
            it.add(5);
        }
    }
    
    come_join(thread);
    come_join(thread2);
    
    return 0;
}
```

```
#include <neo-c.h>
#include <neo-c-pthread.h>

int main(int argc,char** argv)
{
    var li = new come_mutex<list<int>*%>([1,2,3]);
    
    var thread2 = come {
        sleep(3);
        var it = li.lock();
        
        it.to_string().puts();
        
        li.unlock();
    }
    
    var thread = come {
        li.sync() {
            it.add(4);
        }
        li.sync() {
            it.add(5);
        }
    }
    
    come_join(thread);
    come_join(thread2);
    
    return 0;
}
```

```
#include <neo-c.h>
#include <neo-c-pthread.h>

int main(int argc,char** argv)
{
    var li = new come_mutex<list<int>*%>([1,2,3]);
    
    var thread2 = come {
        li.lock().to_string().puts();
        
        sleep(3);
        
        li.lock().to_string().puts();  // on_drop call unlock()
    }
    
    var thread = come {
        li.lock().add(4); // on_drop call unlock()
        li.lock().add(5); // on_drop call unlock()
    }
    
    come_join(thread);
    come_join(thread2);
    
    return 0;
}
```

```
#include <neo-c.h>
#include <neo-c-pthread.h>

int main(int argc,char** argv)
{
    var li = new come_mutex<list<int>*%>([1,2,3]);
    
    var thread2 = come {
        for(int i=0; i<10; i++) {
            li.lock().to_string().puts();
            sleep(1);
        }
    }
    
    var thread = come {
        for(int i=0; i<3; i++) {
            li.lock().add(4);
            sleep(1);
            li.lock().add(5);
            sleep(1);
            li.lock().add(6);
            sleep(1);
            li.lock().add(7);
            sleep(1);
        }
    }
    
    come_join(thread);
    come_join(thread2);
    
    return 0;
}
```

# Here document

```C
#include <neo-c.h>

int main(int argc, char** argv)
{
    int a = 123;
    printf("""
AAA
\{a}
BBB
CCC
""");
    
    return 0;
}
```

# C++

If API is C++ evironment, you can call C++ code like this.

```
c_include {# include <iostream>}
c_include {# include <list>}

int main(){
    int a = 7;
    __type__(std::list<int>) lst;

    lst.push_back(a);
    lst.push_back(2);
    lst.push_back(3);

    __c__ {
        for (int v : lst) {
            std::cout << v << std::endl;
        }
    }

    return 0;
}
```

```
c_include {# include <iostream>}
c_include {# include <list>}

int main(){
    int a = 7;
    __c__ {
    std::list<int>) lst;

    lst.push_back(a);
    lst.push_back(2);
    lst.push_back(3);

    for (int v : lst) {
        std::cout << v << std::endl;
    }
    }

    return 0;
}
```

Well, I know it's nonsense.


# Payload enum

Rust like payload enum syntax is available.

Rust風のpayload付きenumが使えます。

```C
enum Option<T> {
    Some(T),
    None,
};

var some = new Option<int>.Some(123);
var none = new Option<int>.None();

xassert("some", some.is_Some());
xassert("none", none.is_None());
xassert("payload", some.get_Some() == 123);
```

`new Type.Variant(...)` creates a value of that variant.
`is_Variant()` is generated for every variant.
`get_Variant()` is generated when the variant has one payload value.

`new Type.Variant(...)` でそのvariantの値を作れます。
`is_Variant()` は全variantに自動生成されます。
`get_Variant()` はpayloadが1つのvariantに対して自動生成されます。

# Optional collection access

`list<T>`, `vector<T>`, and `map<K,V>` keep the existing `[]` behavior in normal expressions.
If an index or key is out of range, the result is still a zero-cleared value, so existing source code keeps working.

`list<T>`, `vector<T>`, `map<K,V>` の通常の `[]` は従来通りです。
範囲外アクセスや存在しないキーでは0クリアされた値が返るため、既存のソースはそのまま動きます。

```C
var li = [10,20,30];
xassert("normal list fallback", li[100] == 0);

var vec = [1,2,3].to_vector();
xassert("normal vector fallback", vec[100] == 0);

var ma = [1:10, 2:20];
xassert("normal map fallback", ma[3] == 0);
```

When the `[]` expression is used as the direct target of `.case`, neo-c uses an optional load instead.
The value is `Result<T>`. Found elements become `Some(value)`, and out-of-range indexes or missing keys become `None`.

`[]` 式を `.case` の直接の対象にした場合だけ、neo-cはoptional loadを使います。
値は `Result<T>` です。要素が見つかれば `Some(value)`、範囲外インデックスや存在しないキーなら `None` になります。

```C
var li = [10,20,30];

li[1].case {
    (Value.is_Some()) {
        xassert("list some", Value.get_Some() == 20);
    }
    (Value.is_None()) {
        xassert("list none", false);
    }
}

li[100].case {
    (Value.is_Some()) {
        xassert("list some", false);
    }
    (Value.is_None()) {
        xassert("list none", true);
    }
}
```

```C
var ma = [1:10, 2:20];

ma[2].case {
    (Value.is_Some()) {
        xassert("map some", Value.get_Some() == 20);
    }
    (Value.is_None()) {
        xassert("map none", false);
    }
}

ma[3].case {
    (Value.is_Some()) {
        xassert("map some", false);
    }
    (Value.is_None()) {
        xassert("map none", true);
    }
}
```

`.catch { ... }` is available as sugar for the common fallback case.
For `Some(value)`, `.catch` returns `value` and does not run the block.
For `None`, `.catch` runs the block and returns the block result.

`.catch { ... }` はよく使うfallback用の糖衣構文です。
`Some(value)` ならブロックを実行せずに `value` を返します。
`None` ならブロックを実行し、ブロックの戻り値を返します。

```C
var li = [10,20,30];

int a = li[2].catch {
    999
};
xassert("some unwrap", a == 30);

int b = li[100].catch {
    123
};
xassert("none fallback", b == 123);
```

`[]!` is available when you want failure to panic instead of returning the zero-cleared fallback value.
For `Some(value)`, it returns `value`. For `None`, it panics through `Result<T>.unwrap()`.

`[]!` は失敗時に0クリア値ではなくpanicさせたい場合に使えます。
`Some(value)` なら `value` を返し、`None` なら `Result<T>.unwrap()` でpanicします。

```C
var li = [1,2,3];

int n = li[1]!;
xassert("bang unwrap", n == 2);

li[4]!;     // panic
```

## Specification

- Applies to `list<T>`, `vector<T>`, and `map<K,V>`.
- Normal `xs[index]` and `map[key]` access is unchanged and returns a zero-cleared value on failure.
- Only a `[]` expression used directly as `.case` target returns `Result<T>` with `Some` or `None`.
- A `[]` expression used directly as `.catch` target also uses optional load.
- A `[]` expression followed directly by `!` uses optional load and panics when the result is `None`.
- `.catch` requires the catch block to return a value.
- `list` and `vector` keep negative index support in optional loads.
- Heap payloads follow payload enum ownership rules.

- 対象は `list<T>`, `vector<T>`, `map<K,V>` です。
- 通常の `xs[index]` や `map[key]` は変更されず、失敗時は0クリア値を返します。
- `.case` の直接の対象になった `[]` 式だけが `Some` / `None` を持つ `Result<T>` を返します。
- `.catch` の直接の対象になった `[]` 式もoptional loadを使います。
- `[]` 式の直後に `!` を付けた場合もoptional loadを使い、`None` ならpanicします。
- `.catch` のブロックは値を返す必要があります。
- optional loadでも `list` と `vector` の負インデックスは使えます。
- heap payloadはpayload enumの所有権規則に従います。

# Fixed-size C Array Bounds Checks

Known fixed-size C array access is checked at runtime for both `[]` loads and `[]` assignments.
If the index is negative or greater than or equal to the declared bound, neo-c prints `array index out of bounds` and exits with status 2.
Pointer indexing remains normal C-style indexing because the compiler does not know a reliable bound.

固定長C配列のサイズが分かっている場合、`[]` での読み出しと `[]` への代入は実行時に境界チェックされます。
インデックスが負、または宣言された上限以上の場合は `array index out of bounds` を出して exit(2) します。
ポインタ経由の添字アクセスは信頼できる上限が分からないため、従来どおり通常のCの添字アクセスです。

```C
int xs[3] = { 10, 20, 30 };

int a = xs[2];  // ok
xs[1] = 25;     // ok
xs[3] = 99;     // panic
```

# Result<T>

```
#include <neo-c.h>

Result<FILE*>*% xfopen(const char* file_name, const char* mode)
{
    FILE* f = fopen(file_name, mode);

    if(f == NULL) {
        return new Result<FILE*>.None();
    }

    return new Result<FILE*>.Some(f);
}

int main(int argc, char** argv)
{
    xfopen("01main.nc", mode:"r").unwrap().fclose()!;
    xfopen("1main.nc", mode:"r").catch {
        puts("ERR");
        return 1;
    }.fclose()!;

    return 0;
}
```

`Result<T>` is the standard result type. It is a payload enum with `Some(T)` and `None`.

`Result<T>` が標準のresult型です。`Some(T)` と `None` を持つpayload enumです。

The old tuple2-based `RESULT(T)`, `SOME(value)`, and `NONE(value)` macros are removed.
Use `Result<T>*%`, `new Result<T>.Some(value)`, and `new Result<T>.None()` instead.

古い tuple2 ベースの `RESULT(T)`, `SOME(value)`, `NONE(value)` マクロは廃止されました。
代わりに `Result<T>*%`, `new Result<T>.Some(value)`, `new Result<T>.None()` を使います。

| old | new |
| --- | --- |
| `RESULT(int)` | `Result<int>*%` |
| `SOME(10)` | `new Result<int>.Some(10)` |
| `NONE(0)` | `new Result<int>.None()` |

`.catch { ... }` handles the `None` branch and returns the block result. If the value is `Some`, `.catch` unwraps and returns the payload without running the block.

`.catch { ... }` は `None` 側を処理し、ブロックの結果を返します。値が `Some` ならブロックを実行せずpayloadを取り出して返します。

```C
#include <neo-c.h>

Result<int>*% get_int(bool ok)
{
    if(ok) {
        return new Result<int>.Some(10);
    }

    return new Result<int>.None();
}

int ignore_error(bool ok)
{
    int n = get_int(ok).catch {
        77
    };

    return n + 1;
}
```

`result??` is also available for propagation. It unwraps the `Some` value and returns `None` from the current function when the source is `None`.
It uses `??` instead of `?` to avoid ambiguity with the conditional operator.

`result??` による伝播も使えます。元の値が `None` なら現在の関数から `None` を返し、`Some` ならpayloadを取り出します。
条件演算子との曖昧さを避けるため、`?` ではなく `??` にしています。

`??` is a postfix operator.
You can use it only inside a function that returns `Result<T>*%`.
When the source result is success, `expr??` becomes the unwrapped value.
When the source result is error, the current function returns an error result immediately.

`??` は後置演算子です。
`Result<T>*%` を返す関数の中で使えます。
成功時は `expr??` が中身の値になります。
失敗時は現在の関数から直ちにエラー結果を返します。

Typical form:

典型的な書き方:

```C
Result<int>*% fun()
{
    int value = may_fail()??;
    return new Result<int>.Some(value + 1);
}
```

```C
#include <neo-c.h>

Result<FILE*>*% xfopen2(const char* file_name, const char* mode)
{
    FILE* f = fopen(file_name, mode);

    if(f == NULL) {
        return new Result<FILE*>.None();
    }

    return new Result<FILE*>.Some(f);
}

Result<int>*% read_first_byte(const char* file_name)
{
    FILE* f = xfopen2(file_name, "r")??;
    int ch = fgetc(f);
    
    f.fclose()!;
    
    if(ch == EOF) {
        return new Result<int>.None();
    }
    
    return new Result<int>.Some(ch);
}
```

You can also chain multiple `??` calls.

`??` は複数回つなげて使うこともできます。

```C
#include <neo-c.h>

Result<FILE*>*% xfopen2(const char* file_name, const char* mode)
{
    FILE* f = fopen(file_name, mode);
    
    if(f == NULL) {
        return new Result<FILE*>.None();
    }
    
    return new Result<FILE*>.Some(f);
}

Result<char*>*% read_line2(FILE* f)
{
    char* buf = calloc(1, 128);
    
    if(fgets(buf, 128, f) == NULL) {
        free(buf);
        return new Result<char*>.None();
    }
    
    return new Result<char*>.Some(buf);
}

Result<int>*% read_line_len(const char* file_name)
{
    FILE* f = xfopen2(file_name, "r")??;
    char* line = read_line2(f)??;
    
    int len = strlen(line);
    
    free(line);
    fclose(f);
    
    return new Result<int>.Some(len);
}
```

# span, ref , opt

```
#include <neo-c.h>

struct sData
{
    int a;
    int b;
    int c;
};

sData? fun()
{
    sData? p = opt new sData { a:111, b:222, c:333 };
    
    return p;
}

int main(int argc, char** argv)
{
    sData? p = fun();
    
    printf("p.a %d\n", p.a);
    
    return 0;
}
```
p.a 111

optional is owned.

```
#include <neo-c.h>

struct sData
{
    int a;
    int b;
    int c;
};

int main(int argc, char** argv)
{
    sData? p = null;
    
    printf("p.a %d\n", p.a);
    
    return 0;
}
```
null pointer exception. self is null
stackframe
main

```
#include <neo-c.h>

int main(int argc, char** argv) 
{
    int? p = opt new int(5);
    
    printf("%d\n", *p);
                                   
    return 0;
}
```
5.



```
#include <neo-c.h>

struct sData
{
    int a;
    int b;
    int c;
};

sData& fun()
{
    var x = new sData { a:111, b:222, c:333 };

    sData& p = ref borrow x;

    return p;
}

int main(int argc, char** argv)
{
    sData& p = fun();

    printf("p.a %d\n", p.a);

    return 0;
}
```
refferenced heap object is vanished
allocated at a.nc 14
stackframe2
main

```
#include <neo-c.h>

struct sData
{
    int a;
    int b;
    int c;
};

int main(int argc, char** argv)
{
    var x = v[1,2,3];
    
    sData{} p = span borrow x;
    
    printf("p.a %d\n", p.a);
    
    return 0;
}
```
p.a 1
```
#include <neo-c.h>

int main(int argc, char** argv) 
{
    char*% s = string("abc");
    char{} p = span borrow s;
    
    *p = 'X';
    
    puts(p!);
                                   
    return 0;
}
```
Xbc
```
#include <neo-c.h>

int main(int argc, char** argv) 
{
    buffer*% buf = b"ABC";
    char{} p = span borrow buf;
    
    *p = 'X';
    
    puts(p!);
                                   
    return 0;
}
```
XBC

```
#include <neo-c.h>

struct sData
{
    int a;
    int b;
    int c;
};

sData gData = { 1, 2, 3 };

sData{} fun()
{
    sData{} p = span &gData;
    
    return p;
}    

int main(int argc, char** argv) 
{
    var p = fun();
    
    printf("%d %d %d\n", p!.a, p!.b, p!.c);
                                   
    return 0;
}
```

# ZERO COST ITERATOR

neo-c has a Rust-like iterator DSL for `list<T>`, `vector<T>`, and `map<K,V>`.

Use backticks for iterator methods:

```
var li2 = [1,2,3,4,5].`iter().`filter { it % 2 == 1 }.`map { it * 10 }.`collect();
```

Basic rules:

- `iter()` starts the iterator pipeline.
- `it` is the current item inside each block.
- The source expression is evaluated only once.
- `list<T>` and `vector<T>` iterate values.
- `map<K,V>` iterates keys, not values.
- `collect()` materializes the pipeline result.
- Iterator DSL methods use backticks. Normal collection methods such as `count()` do not.

Example:

```
#include <neo-c.h>

int main(int argc, char** argv)
{
    var li = [1,2,3,4,5,6,7];
    
    var li2 = li.`iter().`filter { it > 3 }.`map { it.to_string() + "B" }.`collect();
    
    puts(li2.to_string());   // [4B, 5B, 6B, 7B]
    
    return 0;
}
```

Iterator DSL methods:

`iter()`

- Starts the pipeline.
- Available on `list<T>`, `vector<T>`, and `map<K,V>`.
- Evaluates the source once before iteration begins.

```
var li = [1,2,3].`iter().`collect();
```

`filter { ... }`

- Keeps items where the block returns `true`.

```
var li = [1,2,3,4].`iter().`filter { it % 2 == 0 }.`collect();   // [2,4]
```

`map { ... }`

- Replaces `it` with the block result.
- Use this to transform item type or value.

```
var li = [1,2,3].`iter().`map { it.to_string() + "!" }.`collect();   // [1!,2!,3!]
```

`take(n)`

- Yields only the first `n` items.

```
var li = [1,2,3,4,5].`iter().`take(3).`collect();   // [1,2,3]
```

`skip(n)`

- Skips the first `n` items, then yields the rest.

```
var li = [1,2,3,4,5].`iter().`skip(2).`collect();   // [3,4,5]
```

`take_while { ... }`

- Yields items while the block returns `true`.
- Stops at the first `false`.

```
var li = [1,2,3,4,1].`iter().`take_while { it < 4 }.`collect();   // [1,2,3]
```

`skip_while { ... }`

- Skips items while the block returns `true`.
- Once the block returns `false`, yields that item and all remaining items.

```
var li = [1,2,3,2].`iter().`skip_while { it < 3 }.`collect();   // [3,2]
```

`step_by(n)`

- Yields every `n`th item starting from index `0`.

```
var li = [1,2,3,4,5,6].`iter().`step_by(2).`collect();   // [1,3,5]
```

`cloned()`

- Clones each item before passing it to the next stage.
- Useful when you want an owned value in later stages.

```
var li = [1,2,3].`iter().`cloned().`map { it + 10 }.`collect();   // [11,12,13]
```

`copied()`

- Produces a copied item for the next stage.
- Current behavior is the same style as `cloned()` in the shipped implementation.

```
var li = [1,2,3].`iter().`copied().`map { it + 20 }.`collect();   // [21,22,23]
```

`enumerate()`

- Rebinds `it` to a tuple of `(index, value)`.
- Access fields with `it.v1`, `it.v2`, and so on.

```
var li = [10,20,30].`iter().`enumerate().`map { s"\{it.v1}:\{it.v2}" }.`collect();
// [0:10,1:20,2:30]
```

`inspect { ... }`

- Runs the block for side effects and keeps the original item flowing.

```
int sum = 0;
var li = [1,2,3].`iter().`inspect { sum += it; }.`collect();   // [1,2,3]
```

`find { ... }`

- Keeps the first item where the block returns `true`.
- Stops the pipeline after that item.
- If nothing matches, `collect()` returns an empty container.

```
var li = [1,2,3,4,5].`iter().`find { it > 3 }.`collect();   // [4]
```

`each { ... }`

- Terminal operation for side effects.
- Runs the block for each yielded item.
- Alias-style terminal similar to `for_each()`.

```
[1,2,3].`iter().`each {
    printf("%d\n", it);
};
```

`for_each { ... }`

- Terminal operation for side effects.
- Runs the block for each yielded item.

```
[1,2,3].`iter().`for_each {
    printf("%d\n", it);
};
```

`collect()`

- Terminal operation that materializes the result.
- `list<T>.`iter()...`collect()` returns `list<T2>`.
- `vector<T>.`iter()...`collect()` returns `vector<T2>`.
- `map<K,V>.`iter()...`collect()` returns `list<K2>` because map iteration flows keys.

```
var li = [1,2,3].`iter().`map { it * 2 }.`collect();   // [2,4,6]
```

`end()`

- Terminal operation that finishes the pipeline without collecting values.
- Use this when only side effects matter and you do not want a result container.

```
[1,2,3].`iter().`inspect { printf("%d\n", it); }.`end();
```

Normal collection terminal methods:

These are not backtick iterator operators. They are normal methods on `list<T>`, `vector<T>`, and `map<K,V>`, so you call them after a collection-producing step such as `filter { ... }` or `map { ... }`.

`count()`

- Returns the number of items.

```
[1,2,3,4].filter { it % 2 == 0 }.count();   // 2
```

`any { ... }`

- Returns `true` if any item matches.
- Returns `false` on an empty collection.

```
[1,2,3].any { it == 2 };   // true
```

`all { ... }`

- Returns `true` if all items match.
- Returns `true` on an empty collection.

```
[1,2,3].all { it < 4 };   // true
```

`position(default_value) { ... }`

- Returns the zero-based index of the first matching item.
- Returns `default_value` if no item matches.

```
[3,1,4].position(-1) { it == 1 };   // 1
```

`find_value(default_value) { ... }`

- Returns the first matching item itself.
- Returns `default_value` if no item matches.

```
[3,1,4].find_value(-1) { it > 3 };   // 4
```

`nth(index, default_value)`

- Returns the item at `index`.
- Negative indices count from the end.
- Returns `default_value` if out of range.

```
[10,20,30].nth(1, -1);    // 20
[10,20,30].nth(-1, -1);   // 30
```

`last(default_value)`

- Returns the last item.
- Returns `default_value` for an empty collection.

```
[10,20,30].last(-1);   // 30
```

`sum()`

- Returns the sum of all items.
- Returns `0` for an empty collection.

```
[1,2,3,4].sum();   // 10
```

`product()`

- Returns the product of all items.
- Returns `1` for an empty collection.

```
[1,2,3,4].product();   // 24
```

`min(default_value)`

- Returns the minimum item.
- Returns `default_value` for an empty collection.

```
[3,1,4,2].min(-1);   // 1
```

`max(default_value)`

- Returns the maximum item.
- Returns `default_value` for an empty collection.

```
[3,1,4,2].max(-1);   // 4
```

Notes:

- Use `it.to_string() + "B"` for string output.
- `it + "B"` is invalid because `int + string` is not allowed.
- `map<K,V>` iterator pipelines and terminal methods operate on keys.
