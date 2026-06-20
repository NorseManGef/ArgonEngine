#GENERATED FILE! DO NOT EDIT BY HAND!
set(WAYLAND_PROTOCOLS_DIR
${GENERATED_DIR}/../../linux/SDL3/
)
set(XDG_FOREIGN_UNSTABLE_V2_XML ${WAYLAND_PROTOCOLS_DIR}/wayland-protocols/xdg-foreign-unstable-v2.xml)
add_custom_command(
OUTPUT
${GENERATED_DIR}/xdg-foreign-unstable-v2-client-protocol.h ${GENERATED_DIR}/xdg-foreign-unstable-v2-protocol.c
COMMAND wayland-scanner client-header
${XDG_FOREIGN_UNSTABLE_V2_XML}
${GENERATED_DIR}/xdg-foreign-unstable-v2-client-protocol.h
COMMAND wayland-scanner private-code
${XDG_FOREIGN_UNSTABLE_V2_XML}
${GENERATED_DIR}/xdg-foreign-unstable-v2-protocol.c
DEPENDS ${XDG_FOREIGN_UNSTABLE_V2_XML}
)
list(APPEND SRC_SDL3 ${GENERATED_DIR}/xdg-foreign-unstable-v2-protocol.c)
set(CURSOR_SHAPE_V1_XML ${WAYLAND_PROTOCOLS_DIR}/wayland-protocols/cursor-shape-v1.xml)
add_custom_command(
OUTPUT
${GENERATED_DIR}/cursor-shape-v1-client-protocol.h ${GENERATED_DIR}/cursor-shape-v1-protocol.c
COMMAND wayland-scanner client-header
${CURSOR_SHAPE_V1_XML}
${GENERATED_DIR}/cursor-shape-v1-client-protocol.h
COMMAND wayland-scanner private-code
${CURSOR_SHAPE_V1_XML}
${GENERATED_DIR}/cursor-shape-v1-protocol.c
DEPENDS ${CURSOR_SHAPE_V1_XML}
)
list(APPEND SRC_SDL3 ${GENERATED_DIR}/cursor-shape-v1-protocol.c)
set(XDG_DECORATION_UNSTABLE_V1_XML ${WAYLAND_PROTOCOLS_DIR}/wayland-protocols/xdg-decoration-unstable-v1.xml)
add_custom_command(
OUTPUT
${GENERATED_DIR}/xdg-decoration-unstable-v1-client-protocol.h ${GENERATED_DIR}/xdg-decoration-unstable-v1-protocol.c
COMMAND wayland-scanner client-header
${XDG_DECORATION_UNSTABLE_V1_XML}
${GENERATED_DIR}/xdg-decoration-unstable-v1-client-protocol.h
COMMAND wayland-scanner private-code
${XDG_DECORATION_UNSTABLE_V1_XML}
${GENERATED_DIR}/xdg-decoration-unstable-v1-protocol.c
DEPENDS ${XDG_DECORATION_UNSTABLE_V1_XML}
)
list(APPEND SRC_SDL3 ${GENERATED_DIR}/xdg-decoration-unstable-v1-protocol.c)
set(XDG_ACTIVATION_V1_XML ${WAYLAND_PROTOCOLS_DIR}/wayland-protocols/xdg-activation-v1.xml)
add_custom_command(
OUTPUT
${GENERATED_DIR}/xdg-activation-v1-client-protocol.h ${GENERATED_DIR}/xdg-activation-v1-protocol.c
COMMAND wayland-scanner client-header
${XDG_ACTIVATION_V1_XML}
${GENERATED_DIR}/xdg-activation-v1-client-protocol.h
COMMAND wayland-scanner private-code
${XDG_ACTIVATION_V1_XML}
${GENERATED_DIR}/xdg-activation-v1-protocol.c
DEPENDS ${XDG_ACTIVATION_V1_XML}
)
list(APPEND SRC_SDL3 ${GENERATED_DIR}/xdg-activation-v1-protocol.c)
set(WAYLAND_XML ${WAYLAND_PROTOCOLS_DIR}/wayland-protocols/wayland.xml)
add_custom_command(
OUTPUT
${GENERATED_DIR}/wayland-client-protocol.h ${GENERATED_DIR}/wayland-protocol.c
COMMAND wayland-scanner client-header
${WAYLAND_XML}
${GENERATED_DIR}/wayland-client-protocol.h
COMMAND wayland-scanner private-code
${WAYLAND_XML}
${GENERATED_DIR}/wayland-protocol.c
DEPENDS ${WAYLAND_XML}
)
list(APPEND SRC_SDL3 ${GENERATED_DIR}/wayland-protocol.c)
set(PRIMARY_SELECTION_UNSTABLE_V1_XML ${WAYLAND_PROTOCOLS_DIR}/wayland-protocols/primary-selection-unstable-v1.xml)
add_custom_command(
OUTPUT
${GENERATED_DIR}/primary-selection-unstable-v1-client-protocol.h ${GENERATED_DIR}/primary-selection-unstable-v1-protocol.c
COMMAND wayland-scanner client-header
${PRIMARY_SELECTION_UNSTABLE_V1_XML}
${GENERATED_DIR}/primary-selection-unstable-v1-client-protocol.h
COMMAND wayland-scanner private-code
${PRIMARY_SELECTION_UNSTABLE_V1_XML}
${GENERATED_DIR}/primary-selection-unstable-v1-protocol.c
DEPENDS ${PRIMARY_SELECTION_UNSTABLE_V1_XML}
)
list(APPEND SRC_SDL3 ${GENERATED_DIR}/primary-selection-unstable-v1-protocol.c)
set(INPUT_TIMESTAMPS_UNSTABLE_V1_XML ${WAYLAND_PROTOCOLS_DIR}/wayland-protocols/input-timestamps-unstable-v1.xml)
add_custom_command(
OUTPUT
${GENERATED_DIR}/input-timestamps-unstable-v1-client-protocol.h ${GENERATED_DIR}/input-timestamps-unstable-v1-protocol.c
COMMAND wayland-scanner client-header
${INPUT_TIMESTAMPS_UNSTABLE_V1_XML}
${GENERATED_DIR}/input-timestamps-unstable-v1-client-protocol.h
COMMAND wayland-scanner private-code
${INPUT_TIMESTAMPS_UNSTABLE_V1_XML}
${GENERATED_DIR}/input-timestamps-unstable-v1-protocol.c
DEPENDS ${INPUT_TIMESTAMPS_UNSTABLE_V1_XML}
)
list(APPEND SRC_SDL3 ${GENERATED_DIR}/input-timestamps-unstable-v1-protocol.c)
set(ALPHA_MODIFIER_V1_XML ${WAYLAND_PROTOCOLS_DIR}/wayland-protocols/alpha-modifier-v1.xml)
add_custom_command(
OUTPUT
${GENERATED_DIR}/alpha-modifier-v1-client-protocol.h ${GENERATED_DIR}/alpha-modifier-v1-protocol.c
COMMAND wayland-scanner client-header
${ALPHA_MODIFIER_V1_XML}
${GENERATED_DIR}/alpha-modifier-v1-client-protocol.h
COMMAND wayland-scanner private-code
${ALPHA_MODIFIER_V1_XML}
${GENERATED_DIR}/alpha-modifier-v1-protocol.c
DEPENDS ${ALPHA_MODIFIER_V1_XML}
)
list(APPEND SRC_SDL3 ${GENERATED_DIR}/alpha-modifier-v1-protocol.c)
set(RELATIVE_POINTER_UNSTABLE_V1_XML ${WAYLAND_PROTOCOLS_DIR}/wayland-protocols/relative-pointer-unstable-v1.xml)
add_custom_command(
OUTPUT
${GENERATED_DIR}/relative-pointer-unstable-v1-client-protocol.h ${GENERATED_DIR}/relative-pointer-unstable-v1-protocol.c
COMMAND wayland-scanner client-header
${RELATIVE_POINTER_UNSTABLE_V1_XML}
${GENERATED_DIR}/relative-pointer-unstable-v1-client-protocol.h
COMMAND wayland-scanner private-code
${RELATIVE_POINTER_UNSTABLE_V1_XML}
${GENERATED_DIR}/relative-pointer-unstable-v1-protocol.c
DEPENDS ${RELATIVE_POINTER_UNSTABLE_V1_XML}
)
list(APPEND SRC_SDL3 ${GENERATED_DIR}/relative-pointer-unstable-v1-protocol.c)
set(XDG_SHELL_XML ${WAYLAND_PROTOCOLS_DIR}/wayland-protocols/xdg-shell.xml)
add_custom_command(
OUTPUT
${GENERATED_DIR}/xdg-shell-client-protocol.h ${GENERATED_DIR}/xdg-shell-protocol.c
COMMAND wayland-scanner client-header
${XDG_SHELL_XML}
${GENERATED_DIR}/xdg-shell-client-protocol.h
COMMAND wayland-scanner private-code
${XDG_SHELL_XML}
${GENERATED_DIR}/xdg-shell-protocol.c
DEPENDS ${XDG_SHELL_XML}
)
list(APPEND SRC_SDL3 ${GENERATED_DIR}/xdg-shell-protocol.c)
set(TABLET_V2_XML ${WAYLAND_PROTOCOLS_DIR}/wayland-protocols/tablet-v2.xml)
add_custom_command(
OUTPUT
${GENERATED_DIR}/tablet-v2-client-protocol.h ${GENERATED_DIR}/tablet-v2-protocol.c
COMMAND wayland-scanner client-header
${TABLET_V2_XML}
${GENERATED_DIR}/tablet-v2-client-protocol.h
COMMAND wayland-scanner private-code
${TABLET_V2_XML}
${GENERATED_DIR}/tablet-v2-protocol.c
DEPENDS ${TABLET_V2_XML}
)
list(APPEND SRC_SDL3 ${GENERATED_DIR}/tablet-v2-protocol.c)
set(XDG_DIALOG_V1_XML ${WAYLAND_PROTOCOLS_DIR}/wayland-protocols/xdg-dialog-v1.xml)
add_custom_command(
OUTPUT
${GENERATED_DIR}/xdg-dialog-v1-client-protocol.h ${GENERATED_DIR}/xdg-dialog-v1-protocol.c
COMMAND wayland-scanner client-header
${XDG_DIALOG_V1_XML}
${GENERATED_DIR}/xdg-dialog-v1-client-protocol.h
COMMAND wayland-scanner private-code
${XDG_DIALOG_V1_XML}
${GENERATED_DIR}/xdg-dialog-v1-protocol.c
DEPENDS ${XDG_DIALOG_V1_XML}
)
list(APPEND SRC_SDL3 ${GENERATED_DIR}/xdg-dialog-v1-protocol.c)
set(POINTER_GESTURES_UNSTABLE_V1_XML ${WAYLAND_PROTOCOLS_DIR}/wayland-protocols/pointer-gestures-unstable-v1.xml)
add_custom_command(
OUTPUT
${GENERATED_DIR}/pointer-gestures-unstable-v1-client-protocol.h ${GENERATED_DIR}/pointer-gestures-unstable-v1-protocol.c
COMMAND wayland-scanner client-header
${POINTER_GESTURES_UNSTABLE_V1_XML}
${GENERATED_DIR}/pointer-gestures-unstable-v1-client-protocol.h
COMMAND wayland-scanner private-code
${POINTER_GESTURES_UNSTABLE_V1_XML}
${GENERATED_DIR}/pointer-gestures-unstable-v1-protocol.c
DEPENDS ${POINTER_GESTURES_UNSTABLE_V1_XML}
)
list(APPEND SRC_SDL3 ${GENERATED_DIR}/pointer-gestures-unstable-v1-protocol.c)
set(XDG_OUTPUT_UNSTABLE_V1_XML ${WAYLAND_PROTOCOLS_DIR}/wayland-protocols/xdg-output-unstable-v1.xml)
add_custom_command(
OUTPUT
${GENERATED_DIR}/xdg-output-unstable-v1-client-protocol.h ${GENERATED_DIR}/xdg-output-unstable-v1-protocol.c
COMMAND wayland-scanner client-header
${XDG_OUTPUT_UNSTABLE_V1_XML}
${GENERATED_DIR}/xdg-output-unstable-v1-client-protocol.h
COMMAND wayland-scanner private-code
${XDG_OUTPUT_UNSTABLE_V1_XML}
${GENERATED_DIR}/xdg-output-unstable-v1-protocol.c
DEPENDS ${XDG_OUTPUT_UNSTABLE_V1_XML}
)
list(APPEND SRC_SDL3 ${GENERATED_DIR}/xdg-output-unstable-v1-protocol.c)
set(TEXT_INPUT_UNSTABLE_V3_XML ${WAYLAND_PROTOCOLS_DIR}/wayland-protocols/text-input-unstable-v3.xml)
add_custom_command(
OUTPUT
${GENERATED_DIR}/text-input-unstable-v3-client-protocol.h ${GENERATED_DIR}/text-input-unstable-v3-protocol.c
COMMAND wayland-scanner client-header
${TEXT_INPUT_UNSTABLE_V3_XML}
${GENERATED_DIR}/text-input-unstable-v3-client-protocol.h
COMMAND wayland-scanner private-code
${TEXT_INPUT_UNSTABLE_V3_XML}
${GENERATED_DIR}/text-input-unstable-v3-protocol.c
DEPENDS ${TEXT_INPUT_UNSTABLE_V3_XML}
)
list(APPEND SRC_SDL3 ${GENERATED_DIR}/text-input-unstable-v3-protocol.c)
set(IDLE_INHIBIT_UNSTABLE_V1_XML ${WAYLAND_PROTOCOLS_DIR}/wayland-protocols/idle-inhibit-unstable-v1.xml)
add_custom_command(
OUTPUT
${GENERATED_DIR}/idle-inhibit-unstable-v1-client-protocol.h ${GENERATED_DIR}/idle-inhibit-unstable-v1-protocol.c
COMMAND wayland-scanner client-header
${IDLE_INHIBIT_UNSTABLE_V1_XML}
${GENERATED_DIR}/idle-inhibit-unstable-v1-client-protocol.h
COMMAND wayland-scanner private-code
${IDLE_INHIBIT_UNSTABLE_V1_XML}
${GENERATED_DIR}/idle-inhibit-unstable-v1-protocol.c
DEPENDS ${IDLE_INHIBIT_UNSTABLE_V1_XML}
)
list(APPEND SRC_SDL3 ${GENERATED_DIR}/idle-inhibit-unstable-v1-protocol.c)
set(POINTER_WARP_V1_XML ${WAYLAND_PROTOCOLS_DIR}/wayland-protocols/pointer-warp-v1.xml)
add_custom_command(
OUTPUT
${GENERATED_DIR}/pointer-warp-v1-client-protocol.h ${GENERATED_DIR}/pointer-warp-v1-protocol.c
COMMAND wayland-scanner client-header
${POINTER_WARP_V1_XML}
${GENERATED_DIR}/pointer-warp-v1-client-protocol.h
COMMAND wayland-scanner private-code
${POINTER_WARP_V1_XML}
${GENERATED_DIR}/pointer-warp-v1-protocol.c
DEPENDS ${POINTER_WARP_V1_XML}
)
list(APPEND SRC_SDL3 ${GENERATED_DIR}/pointer-warp-v1-protocol.c)
set(FRACTIONAL_SCALE_V1_XML ${WAYLAND_PROTOCOLS_DIR}/wayland-protocols/fractional-scale-v1.xml)
add_custom_command(
OUTPUT
${GENERATED_DIR}/fractional-scale-v1-client-protocol.h ${GENERATED_DIR}/fractional-scale-v1-protocol.c
COMMAND wayland-scanner client-header
${FRACTIONAL_SCALE_V1_XML}
${GENERATED_DIR}/fractional-scale-v1-client-protocol.h
COMMAND wayland-scanner private-code
${FRACTIONAL_SCALE_V1_XML}
${GENERATED_DIR}/fractional-scale-v1-protocol.c
DEPENDS ${FRACTIONAL_SCALE_V1_XML}
)
list(APPEND SRC_SDL3 ${GENERATED_DIR}/fractional-scale-v1-protocol.c)
set(COLOR_MANAGEMENT_V1_XML ${WAYLAND_PROTOCOLS_DIR}/wayland-protocols/color-management-v1.xml)
add_custom_command(
OUTPUT
${GENERATED_DIR}/color-management-v1-client-protocol.h ${GENERATED_DIR}/color-management-v1-protocol.c
COMMAND wayland-scanner client-header
${COLOR_MANAGEMENT_V1_XML}
${GENERATED_DIR}/color-management-v1-client-protocol.h
COMMAND wayland-scanner private-code
${COLOR_MANAGEMENT_V1_XML}
${GENERATED_DIR}/color-management-v1-protocol.c
DEPENDS ${COLOR_MANAGEMENT_V1_XML}
)
list(APPEND SRC_SDL3 ${GENERATED_DIR}/color-management-v1-protocol.c)
set(POINTER_CONSTRAINTS_UNSTABLE_V1_XML ${WAYLAND_PROTOCOLS_DIR}/wayland-protocols/pointer-constraints-unstable-v1.xml)
add_custom_command(
OUTPUT
${GENERATED_DIR}/pointer-constraints-unstable-v1-client-protocol.h ${GENERATED_DIR}/pointer-constraints-unstable-v1-protocol.c
COMMAND wayland-scanner client-header
${POINTER_CONSTRAINTS_UNSTABLE_V1_XML}
${GENERATED_DIR}/pointer-constraints-unstable-v1-client-protocol.h
COMMAND wayland-scanner private-code
${POINTER_CONSTRAINTS_UNSTABLE_V1_XML}
${GENERATED_DIR}/pointer-constraints-unstable-v1-protocol.c
DEPENDS ${POINTER_CONSTRAINTS_UNSTABLE_V1_XML}
)
list(APPEND SRC_SDL3 ${GENERATED_DIR}/pointer-constraints-unstable-v1-protocol.c)
set(XDG_TOPLEVEL_ICON_V1_XML ${WAYLAND_PROTOCOLS_DIR}/wayland-protocols/xdg-toplevel-icon-v1.xml)
add_custom_command(
OUTPUT
${GENERATED_DIR}/xdg-toplevel-icon-v1-client-protocol.h ${GENERATED_DIR}/xdg-toplevel-icon-v1-protocol.c
COMMAND wayland-scanner client-header
${XDG_TOPLEVEL_ICON_V1_XML}
${GENERATED_DIR}/xdg-toplevel-icon-v1-client-protocol.h
COMMAND wayland-scanner private-code
${XDG_TOPLEVEL_ICON_V1_XML}
${GENERATED_DIR}/xdg-toplevel-icon-v1-protocol.c
DEPENDS ${XDG_TOPLEVEL_ICON_V1_XML}
)
list(APPEND SRC_SDL3 ${GENERATED_DIR}/xdg-toplevel-icon-v1-protocol.c)
set(FROG_COLOR_MANAGEMENT_V1_XML ${WAYLAND_PROTOCOLS_DIR}/wayland-protocols/frog-color-management-v1.xml)
add_custom_command(
OUTPUT
${GENERATED_DIR}/frog-color-management-v1-client-protocol.h ${GENERATED_DIR}/frog-color-management-v1-protocol.c
COMMAND wayland-scanner client-header
${FROG_COLOR_MANAGEMENT_V1_XML}
${GENERATED_DIR}/frog-color-management-v1-client-protocol.h
COMMAND wayland-scanner private-code
${FROG_COLOR_MANAGEMENT_V1_XML}
${GENERATED_DIR}/frog-color-management-v1-protocol.c
DEPENDS ${FROG_COLOR_MANAGEMENT_V1_XML}
)
list(APPEND SRC_SDL3 ${GENERATED_DIR}/frog-color-management-v1-protocol.c)
set(VIEWPORTER_XML ${WAYLAND_PROTOCOLS_DIR}/wayland-protocols/viewporter.xml)
add_custom_command(
OUTPUT
${GENERATED_DIR}/viewporter-client-protocol.h ${GENERATED_DIR}/viewporter-protocol.c
COMMAND wayland-scanner client-header
${VIEWPORTER_XML}
${GENERATED_DIR}/viewporter-client-protocol.h
COMMAND wayland-scanner private-code
${VIEWPORTER_XML}
${GENERATED_DIR}/viewporter-protocol.c
DEPENDS ${VIEWPORTER_XML}
)
list(APPEND SRC_SDL3 ${GENERATED_DIR}/viewporter-protocol.c)
set(KEYBOARD_SHORTCUTS_INHIBIT_UNSTABLE_V1_XML ${WAYLAND_PROTOCOLS_DIR}/wayland-protocols/keyboard-shortcuts-inhibit-unstable-v1.xml)
add_custom_command(
OUTPUT
${GENERATED_DIR}/keyboard-shortcuts-inhibit-unstable-v1-client-protocol.h ${GENERATED_DIR}/keyboard-shortcuts-inhibit-unstable-v1-protocol.c
COMMAND wayland-scanner client-header
${KEYBOARD_SHORTCUTS_INHIBIT_UNSTABLE_V1_XML}
${GENERATED_DIR}/keyboard-shortcuts-inhibit-unstable-v1-client-protocol.h
COMMAND wayland-scanner private-code
${KEYBOARD_SHORTCUTS_INHIBIT_UNSTABLE_V1_XML}
${GENERATED_DIR}/keyboard-shortcuts-inhibit-unstable-v1-protocol.c
DEPENDS ${KEYBOARD_SHORTCUTS_INHIBIT_UNSTABLE_V1_XML}
)
list(APPEND SRC_SDL3 ${GENERATED_DIR}/keyboard-shortcuts-inhibit-unstable-v1-protocol.c)
