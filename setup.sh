#! /bin/bash

# the list of all plugins in this repo
PLUGINS=(GameItems GameplayAbilityItems)

dev() {
    if ! [[ $1 ]]; then
        echo -e "usage: setup.sh dev [PLUGINS_PATH] (e.g. 'MyProject/Plugins')"
        return
    fi

    plugins_path=`realpath $1`
    if ! [[ -d "$plugins_path" ]]; then
        echo "Project directory does not exist: $plugins_path"
        echo -e "usage: setup.sh dev [PLUGINS_PATH]"
        return
    fi

    # symlink all the plugins in the target directory
    echo "Installing for development in '${plugins_path}'"
    for plugin in ${PLUGINS[@]}; do
        src_dir="`pwd`/Plugins/${plugin}"
        dst_dir="${plugins_path}/${plugin}"
        link $src_dir $dst_dir
    done
}


ALL_COMMANDS="dev"


# setup.sh utils
# --------------

# simple cross-platform symlink util
link() {
    # use mklink if on windows
    if [[ -n "$WINDIR" ]]; then
        # determine if the link is a directory
        # also convert '/' to '\'
        if [[ -d "$1" ]]; then
            cmd <<< "mklink /D \"`cygpath -w \"$2\"`\" \"`cygpath -w \"$1\"`\""
        else
            cmd <<< "mklink \"`cygpath -w \"$2\"`\" \"`cygpath -w \"$1\"`\""
        fi
    else
        ln -sf "$1" "$2"
    fi
}

# run command by name
if [[ "$1" ]]; then
    cd $(dirname "$0")
    $1 ${@:2}
else
    echo -e "usage: setup.sh [COMMAND]\n  $ALL_COMMANDS"
fi
