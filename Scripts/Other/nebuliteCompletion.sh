# Remove any existing completion for Nebulite
complete -r Nebulite Nebulite_Debug Nebulite_Coverage 2>/dev/null

_nebulite_completion() {
    local cmd="${COMP_WORDS[0]}"

    case "$(basename "$cmd")" in
        Nebulite|Nebulite_Debug|Nebulite_Coverage)
            ;;
        *)
            return
            ;;
    esac

    [[ -x "$cmd" ]] || return

    local IFS=$'\n'
    COMPREPLY=($("$cmd" __complete__ "${COMP_WORDS[@]:1}"))
}

# Default + explicit
complete -D -F _nebulite_completion
#complete -F _nebulite_completion Nebulite Nebulite_Debug Nebulite_Coverage
complete -F _nebulite_completion "./bin/Nebulite" "./bin/Nebulite_Debug" "./bin/Nebulite_Coverage"
