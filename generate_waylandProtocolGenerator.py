from pathlib import Path

wpPath = Path("linux/SDL3/wayland-protocols").glob("*.xml")

with open("linux/cmake/waylandProtocolGenerator.cmake", "w") as f:
    def createGenerator(i:Path):
        xmlRelPath = Path(*i.parts[i.parts.index("wayland-protocols"):])

        xmlSetVar = i.name.upper().replace('-','_').replace('.','_')
        xmlVar = '${' + xmlSetVar + '}'
        cProth = '${GENERATED_DIR}/' + i.name.split('.')[0] + '-client-protocol.h'
        protc = '${GENERATED_DIR}/' + i.name.split('.')[0] + '-protocol.c'

        _ = f.write(f'set({xmlSetVar} ' + '${WAYLAND_PROTOCOLS_DIR}/' + f'{xmlRelPath})\n')
        _ = f.write(
            'add_custom_command(\n' +
            'OUTPUT\n' +
            f'{cProth} ' +
            f'{protc}\n' +
            'COMMAND wayland-scanner client-header\n' +
            f'{xmlVar}\n' + f'{cProth}\n' +
            'COMMAND wayland-scanner private-code\n' +
            f'{xmlVar}\n' + f'{protc}\n' +
            'DEPENDS ' + f'{xmlVar}\n' +
            ')\n'
        )
        _ = f.write(
            f'list(APPEND SRC_SDL3 {protc})\n'
        )

    _ = f.write("#GENERATED FILE! DO NOT EDIT BY HAND!\n")

    _ = f.write(
        "set(WAYLAND_PROTOCOLS_DIR\n" + 
            "${GENERATED_DIR}/../../linux/SDL3/\n" +
        ")\n"
    )

    for i in wpPath:
        createGenerator(i)
