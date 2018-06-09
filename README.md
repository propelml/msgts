## Prerequisites

Get Depot Tools and make sure it's in your path.
http://commondatastorage.googleapis.com/chrome-infra-docs/flat/depot_tools/docs/html/depot_tools_tutorial.html#_setting_up

For linux you need these prereqs:

    sudo apt-get install libgtk-3-dev pkg-config ccache


## Build

Wrapper around the gclient/gn/ninja for end users. Try this first:

    ./build.py

If that doesn't work, or you need more control, try calling gn manually:

    gn gen out/Debug --args='cc_wrapper="ccache" is_debug=true '

Then build with ninja:

    ninja -C out/Debug/ deno


Other useful commands:

    gn args out/Debug/ --list # List build args
    gn args out/Debug/ # Modify args in $EDITOR
