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

    local cur="${COMP_WORDS[COMP_CWORD]}"
    local IFS=$'\n'
    local -a prog_comps file_comps all_comps
    # program-provided completions
    if ! mapfile -t prog_comps < <("$cmd" __complete__ "${COMP_WORDS[@]:1}" 2>/dev/null); then
        prog_comps=()
    fi
    # file/directory completions for the current word
    mapfile -t file_comps < <(compgen -f -- "$cur")
    # append a trailing slash for directory entries
    for i in "${!file_comps[@]}"; do
        [[ -d "${file_comps[i]}" ]] && file_comps[i]+='/'
    done

    # merge unique results (program completions first)
    declare -A seen
    for c in "${prog_comps[@]}" "${file_comps[@]}"; do
        [[ -n "$c" && -z "${seen[$c]}" ]] || continue
        seen[$c]=1
        all_comps+=("$c")
    done

    COMPREPLY=("${all_comps[@]}")

    # Prevent bash from appending a space after directory completions
    for c in "${COMPREPLY[@]}"; do
        [[ "$c" == */ ]] && { compopt -o nospace; break; }
    done
}

# Default + explicit
complete -D -F _nebulite_completion
complete -F _nebulite_completion "./bin/Nebulite" "./bin/Nebulite_Debug" "./bin/Nebulite_Coverage"
