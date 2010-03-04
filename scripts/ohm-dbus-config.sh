#!/bin/sh

error() {
    echo "$*" 1>&2
}

emit_prefix() {
    echo '<!DOCTYPE busconfig PUBLIC'
    echo '"-//freedesktop//DTD D-BUS Bus Configuration 1.0//EN"'
    echo '"http://www.freedesktop.org/standards/dbus/1.0/busconfig.dtd">'
    echo '<busconfig>'
}

emit_suffix() {
    echo '</busconfig>'
}

emit_ohmd_policy() {
    # emit core ohm policy
    echo '  <policy user="root">'
    echo '    <allow own="org.freedesktop.ohm"/>'
    echo '    <allow send_destination="org.freedesktop.ohm"/>'
    echo '    <allow send_interface="org.freedesktop.ohm.Keystore"/>'
    echo '    <allow send_interface="org.freedesktop.ohm.Manager"/>'
    echo '    <allow receive_sender="*"/>'
    echo '  </policy>'
}

emit_plugin_policy() {
    plugin="${1%.dbus}"; plugin="${plugin#/etc/ohm/plugins.d}"
    echo "  # $plugin plugin"
    echo '  <policy context="mandatory">'

    cat $1 | while read key arg1 arg2 extra; do
#        echo "read: [$key $arg1 $arg2]"
        # skip empty lines
        if test -z "$key"; then
            continue
        fi
        # skip invalid lines
        if test -z "$arg1" -o -n "$extra"; then
            error "Invalid input [$key $arg1 $arg2 $extra] from $cfg"
            continue
        fi

	# check rule type, defaulting to allow
        case $key in
            allow|deny) type=$key; tag=$arg1; value=$arg2;;
            *) if test -n "$arg2"; then
                   error "Invalid input [$key $arg1 $arg2 $extra]"
                   continue
               fi
               type=allow; tag=$key; value=$arg1;;
        esac

        # validate key
        case $key in
            own|user|group) ;;
            send_interface|send_member|send_error|send_destination) ;;
            send_type|send_path) ;;
            receive_interface|receive_member|receive_error|receive_sender);;
            receive_type|receive_path);;
            *) error "Invalid key \"$key\""; continue;;
        esac

        echo "    <$type $tag=\"$value\"/>"
    done
    echo '  </policy>'
}


emit_prefix
emit_ohmd_policy
for cfg in /etc/ohm/plugins.d/*.dbus; do
    if test -f $cfg; then
        emit_plugin_policy $cfg
    fi
done
emit_suffix
