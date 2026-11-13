# Rebobina

**English** · [Português (Brasil)](README.pt-BR.md)

**An open-source, modular software project exploring new possibilities for classic digital cameras.**

Rebobina aims to become an independent, extensible **on-camera program**. It draws on knowledge and, where technically and legally suitable, open-source components from [Magic Lantern](https://magiclantern.fm). Our initial target is the **original Canon EOS 5D (2005), firmware 1.1.1**, commonly called the *5D Classic*—**not** the 5D Mark II, III, or IV.

The long-term goal is to provide our own camera interface, photographic tools, and a versioned module system so that features such as [Soup Engine](https://github.com/jiijii0/soup-engine) can eventually be added or updated independently of the main program.

Rebobina is **not** a Magic Lantern installer or a plugin manager for an existing Magic Lantern installation. The intended program would launch from a prepared CompactFlash card alongside Canon's running firmware, without replacing the firmware stored in ROM. An optional installation assistant could come later.

> [!IMPORTANT]
> **Experimental project — not ready for use on a camera.**
>
> Rebobina currently runs **only in development environments on a computer**. It has **not booted on a Canon EOS 5D Classic**. There is no validated on-camera interface, ARM module loader, or installable release. Do **not** copy host executables, experimental files, or ARM object files to your CompactFlash card. Passing host tests does **not** demonstrate hardware safety.

## Current status

The following components work **on the host computer**:

- **Portable C core:** experimental module registry, versioned module API, lifecycle manager, and a Hello example module.
- **Interface simulators:** terminal navigation and a responsive browser interface driven by the portable C core.
- **Module experiments:** JSON manifest validation and loading of trusted Linux `.so` plugins; **not** camera modules.
- **Hardware abstraction:** a deterministic in-memory HAL for files, tasks, and memory allocation, with injected failures, partial writes, and simulated power cycles.
- **Protected output:** temporary-file staging, synchronization, and exclusive publication without overwriting an existing destination. A separate **Linux-only** backend supports actual host files.
- **Soup Engine integration tests:** a pinned upstream source revision with 13 self-tests, synthetic JPEG/film-look processing, and sanitizer checks.
- **CR2 preview tools:** read-only inspection and a Linux utility that applies Soup Engine film profiles to the **embedded JPEG preview** in a CR2, producing a separate JPEG.
- **ARM946E-S toolchain checks:** relocatable ARM object files are built and validated. **They are not executable camera firmware.**
- **Automated testing:** C and Python tests, GitHub Actions, and host-side sanitizer builds.

**Not yet implemented on the camera:** a working Rebobina boot, physical LCD/buttons, a camera-side plugin loader, a validated FAT filesystem backend, or RAW sensor processing as an on-camera module.

A **5DC.111 boot-chain research audit and fail-closed preflight** now distinguish the historical installer load address (`0x00800000`) from the Magic Lantern relocated runtime (`0x00980000`). A mock Canon-continuation test verifies host-only fault paths, **not real firmware handoff**. See [5DC boot-chain research](docs/5DC-BOOT-CHAIN.md).

An **offline-only linked ARM stage0 ELF** is now available with `make offline-bootstrap`. It demonstrates an independent ARM entry and a link with the portable core, **not a Canon bootable program**: no Canon card-boot loader format, firmware handoff, or hardware recovery exists. Never rename or convert the ELF to `AUTOEXEC.BIN`. See [offline bootstrap study](docs/5DC-BOOTSTRAP-OFFLINE.md) (Portuguese).

The automated CR2 integration tests use a **synthetic TIFF/CR2-like envelope**, not a real Canon photograph. The CR2 developer processes the **embedded JPEG preview, not Bayer RAW sensor data**. Real EOS 5D CR2 compatibility is not yet validated.

## Quick start (host development)

For the basic tools you need a **C11 compiler, Make, and Python 3.9+**. These commands do **not** communicate with a camera.

~~~sh
make demo             # build and run the C host prototype
make hal-demo         # demonstrate the in-memory HAL
make test             # run C/Python tests, including HAL tests
make check-modules    # validate JSON module manifests
make audit-platform   # check the static EOS 5D platform inventory
make arm-objects      # build ARM objects (NOT installable firmware)
make offline-bootstrap # link ARM stage0 ELF for inspection ONLY (NOT camera bootable)
make sim-ui           # simulate the interface in a terminal
make web-sim          # start the visual simulator on localhost:8765
make plugin-demo      # load a trusted Linux .so plugin
python3 tools/modules.py list
python3 tools/modules.py new my-extension --name "My extension"
~~~

The last command creates a draft extension manifest and README, **not** a camera binary.

### Visual simulator

~~~sh
make web-sim
~~~

Open **http://127.0.0.1:8765/** in a browser on **the same computer**. Use the trash-can button to open the menu, ↑/↓ to navigate, and SET to select. In **Modules**, SET toggles the Hello example **in the host simulation only**.

You do not need a camera, CompactFlash reader, `.FIR` file, or Magic Lantern installation. The simulator does **not** emulate the DIGIC II processor, VxWorks, or the physical LCD.

Read the [visual simulator guide](docs/SIMULADOR-VISUAL.md) and the [development plan without a CF reader](docs/SPRINT-SEM-CF.md) (currently in Portuguese).

## Develop a CR2 JPEG preview on Linux

The host utility `rebobina-cr2-develop` follows this pipeline:

1. Open an input CR2 **read-only**, with a 64 MiB size limit.
2. Locate and bounds-check its embedded JPEG preview using Soup Engine's parser.
3. Decode the preview, apply a `.fs` film stock, and encode a **new JPEG**.
4. Publish the result in an explicitly selected output directory using Rebobina's output API and a Linux backend with `openat`, `fsync`, and `renameat2(RENAME_NOREPLACE)`.

Building the Soup tools additionally requires **Linux, Git, Make, a C compiler, Clang, Python, and internet access**. The script fetches a **pinned GPLv2 upstream source revision** into the temporary `build/` directory, applies host-only compatibility adjustments, and runs tests. It does not download or install Canon firmware.

~~~sh
python3 tools/soup_host.py --fetch

mkdir -p ./rebobina-output
./build/rebobina-cr2-develop \
  /path/to/photo.CR2 \
  ./build/soup-upstream/Canon-EOS-5D/src/stocks/portra.fs \
  ./rebobina-output \
  PHOTO-PORTRA.JPG \
  2
~~~

The optional final argument is the output scale: **1** for the embedded preview's original dimensions, or **2** for half its width and height (default). Output names must be safe, short ASCII basenames without directories. **An existing JPEG is never overwritten.** Use a dedicated output directory.

In CI, Rebobina produced a JPEG **byte-for-byte identical** to Soup Engine's reference `previewcheck` program on the **same synthetic CR2-like input** and Soup 800 profile. Tests also reject collisions, truncated input, symbolic-link output directories, and path traversal, while preserving the source. **No real EOS 5D Classic CR2 has been validated yet.**

See the [host CR2 developer guide](docs/CR2-DESENVOLVIMENTO-HOST.md), [protected output protocol](docs/SAIDA-SEGURA.md), and [Linux output backend decision](docs/decisions/0006-hal-posix-segura.md) (Portuguese).

### Inspect an embedded preview without modifying the CR2

~~~sh
./build/rebobina-cr2-inspect /path/to/photo.CR2
~~~

The read-only inspector reports the preview's dimensions, offset, and size, when valid. Success does **not** prove the CR2's authenticity, the integrity of its RAW data, or that the full image is safe to decode. See the [CR2 inspection notes](docs/CR2-INSPECAO-HOST.md).

## Soup Engine tests and known issues

~~~sh
python3 tools/soup_host.py --fetch
~~~

The host test runner compiles and tests a pinned version of the **original Soup Engine** independently, without incorporating it into an on-camera Rebobina module. Current coverage includes:

- The 13 upstream self-tests.
- Synthetic 8×8 image processing with **Soup 800, Portra, and Acros** film stocks.
- Byte-for-byte comparison of complete synthetic JPEG files.
- A synthetic CR2-like envelope containing a real JPEG and tests for corrupt or truncated inputs.
- Fault-injected writes, exclusive output publication, and source-file preservation.
- AddressSanitizer and UndefinedBehaviorSanitizer builds.

Sanitizer testing identified **undefined behavior in upstream JPEG encoding** (left-shifting negative numbers). An experimental test-only copy changes two such expressions to multiplication to extend sanitizer coverage. **The upstream code is not modified or silently integrated into camera firmware.** A separate compiler warning in film-look halation remains under investigation.

Read the [Soup Engine porting notes](docs/SOUP-ADAPTACAO.md) and [sanitizer findings](docs/SOUP-SANITIZERS.md) (Portuguese). These host results do **not** establish on-camera safety.

## What we want to build

- **Independent on-camera runtime:** controlled startup from CompactFlash, task management, native UI, and photographic tools running alongside Canon's original firmware.
- **Modular core:** an API for registering, enabling, disabling, and eventually loading individual modules safely.
- **Extension system:** add-ons such as Soup Engine that can be installed and updated without individually patching the main executable. **Future goal; not implemented on-camera.**
- **Camera-specific backends:** isolate hardware services, starting with the original EOS 5D.
- **Development tooling:** simulation, testing, reproducible builds, and failure injection to reduce risk.
- **Optional assisted installation:** only after on-camera support and recovery procedures have been validated.

We intend to learn from and properly credit Magic Lantern's existing module system instead of needlessly reinventing it. Rebobina will still have its **own identity, interface, architecture, and development roadmap**.

### Repository structure

~~~text
include/rebobina/       Experimental C APIs for modules, HAL, and output
src/core/               Portable module registry, UI, and output logic
src/host/               Linux demos, simulators, CR2 tools, and HAL backends
camera/5DC.111/         Camera research and experimental ARM stubs
modules/hello/          Example host module
modules/soup-engine/    Planned Soup integration manifest
simulator/web/          Local browser interface (HTML/CSS/JavaScript)
tools/                  Simulator server, manifest tools, Soup test runner
tests/                  Automated host tests and synthetic fixtures
docs/                   Architecture, safety, roadmap, and research notes
~~~

### Current milestones

- **Sprint 0 — Foundation:** complete for the host prototype.
- **Sprint 1 — Reverse engineering:** ongoing; the exact historical Magic Lantern bootstrap binary/source and reference hashes for the EOS 5D Classic still need verification.
- **Sprint 2 — Toolchain and simulation:** host simulators, HAL mocks, and relocatable ARM objects. No camera executable.
- **Sprints 4–7 — Early host proofs:** interface, module lifecycle, Soup Engine JPEG preview development, and reliability testing. Hardware acceptance criteria remain open.
- **Sprint 3 — First controlled camera boot:** **not yet tested or validated**.

The physical test camera was last reported to run **Canon firmware 1.1.0**, while the experimental target is **1.1.1**. A CF reader, verified firmware-update provenance, a validated bootstrap, and a recovery plan are needed before a physical boot test. The host processing pipeline also needs a much smaller memory footprint and a verified VxWorks/FAT backend before any camera-side port.

## Initial target hardware

| Property | Target |
| --- | --- |
| Camera | Canon EOS **5D Classic (2005)**; **not** Mark II/III/IV |
| Canon firmware | **1.1.1** |
| Platform | VxWorks / DIGIC II |
| Historical reference | Magic Lantern support for the original EOS 5D |
| Planned first camera module | Soup Engine |
| Current camera support | **None; host prototype only** |

No compatibility with other camera models is promised. Every new port will require separate research and physical validation.

## Documentation

**Language note:** the detailed technical documents linked below are **currently in Brazilian Portuguese**. This English README is the international starting point. Documentation translations and improvements are welcome.

### Architecture and contributing

- [Proposed architecture](docs/ARQUITETURA.md)
- [Safety, boot, and current limitations](docs/SEGURANCA.md)
- [High-level roadmap](docs/ROADMAP.md)
- [Sprints and acceptance criteria](docs/SPRINTS.md)
- [Module contracts and tooling](docs/MODULOS.md)
- [Contributing](CONTRIBUTING.md)
- [Contributor and coding-agent instructions](AGENTS.md)

### Host development, photography, and Soup Engine

- [CR2 JPEG preview development on Linux](docs/CR2-DESENVOLVIMENTO-HOST.md)
- [Read-only CR2 preview inspection](docs/CR2-INSPECAO-HOST.md)
- [Protected JPEG output and temporary-file recovery](docs/SAIDA-SEGURA.md)
- [Simulated HAL API and fault injection](docs/HAL-SIMULADA.md)
- [Soup Engine porting research](docs/SOUP-ADAPTACAO.md)
- [Soup Engine sanitizer findings](docs/SOUP-SANITIZERS.md)
- [Visual simulator guide](docs/SIMULADOR-VISUAL.md)
- [Development without a CF reader](docs/SPRINT-SEM-CF.md)

### Research, sprint reports, and architectural decisions

- [Original EOS 5D platform research](docs/5DC-PLATAFORMA.md)
- [Sprint 1 test report](docs/SPRINT1-TESTES.md)
- [Sprint 1 evidence and references](docs/REFERENCIAS-SPRINT1.md)
- [Sprint 2 ARM and host UI foundation](docs/SPRINT2-BASE.md)
- [Sprint 3 first-boot checklist](docs/SPRINT3-CHECKLIST.md)
- [Sprint 5 Linux module prototype](docs/SPRINT5-PROTOTIPO.md)
- [Sprint 6 Soup Engine port plan](docs/SPRINT6-SOUP-PORT.md)
- [Sprint 7 reliability tests](docs/SPRINT7-TESTES.md)
- [Sprint 8 future distribution](docs/SPRINT8-DISTRIBUICAO.md)
- [Progress report: Sprints 2, 5, and 7](docs/RELATORIO-AVANCO.md)
- [ADR 0002: browser simulator and C core](docs/decisions/0002-simulador-host.md)
- [ADR 0003: deterministic host HAL](docs/decisions/0003-hal-deterministica.md)
- [ADR 0004: exclusive output publication](docs/decisions/0004-publicacao-exclusiva.md)
- [ADR 0005: checking CR2 preview bounds](docs/decisions/0005-cr2-preview-validacao.md)
- [ADR 0006: Linux output backend](docs/decisions/0006-hal-posix-segura.md)

## Credits and license

Rebobina is an independent project **not affiliated with Canon or the Magic Lantern team**. We acknowledge the [Magic Lantern community](https://github.com/reticulatedpines/magiclantern_simplified) and the [Soup Engine contributors](https://github.com/jiijii0/soup-engine).

The host simulator, HALs, and module manager are original Rebobina components. Pinned GPLv2 Soup Engine source code is downloaded into a temporary build directory for testing; **Soup Engine is not integrated into an installable camera release**. Any future reuse must preserve copyright notices, attribution, and license requirements.

Original Rebobina code is licensed under **GPL-2.0-only**—see [LICENSE](LICENSE). Experimental software, provided **without warranty**.
