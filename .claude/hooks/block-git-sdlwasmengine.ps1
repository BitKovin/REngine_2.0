# PreToolUse hook: in the F:\Git\SdlWasmEngine repository, git is allowed except:
#   * pushing (git push, and subcommands that push, e.g. `git subtree push`)
#   * commits / tags / notes / merges whose message mentions Claude, Anthropic or a Co-Authored-By trailer - inline
#     (-m, heredoc) or from a message file (-F / --file)
# Applies when the session's working directory is inside the repo or the command references it by path.
# Exit code 2 blocks the tool call and shows stderr to Claude.
$payload = [Console]::In.ReadToEnd() | ConvertFrom-Json
$cmd = [string]$payload.tool_input.command
$cwd = [string]$payload.cwd
if (-not $cmd) { exit 0 }

$repo = '(?i)git[\\/]+sdlwasmengine'
if (($cwd -notmatch $repo) -and ($cmd -notmatch $repo)) { exit 0 }

$mention = '(?i)claude|anthropic|co-authored-by'

function Block([string]$why) {
    [Console]::Error.WriteLine("Blocked by user hook: $why")
    exit 2
}

# git / git.exe in command position, optionally with a path prefix (e.g. "C:\...\git.exe").
$pattern = '(?i)(?:^|[\s;&|(''"])(?:[^\s;&|''"]*[\\/])?git(?:\.exe)?[''"]?(?=\s|$)(?<rest>[^;&|\r\n]*)'
foreach ($m in [regex]::Matches($cmd, $pattern)) {
    $rest = $m.Groups['rest'].Value.Trim()
    if ($rest -match '(?i)(^|\s)push(\s|$)') { Block "git push is not allowed in the SdlWasmEngine repository - the user pushes." }

    if ($rest -match '(?i)(^|\s)(commit|tag|notes|merge|revert|cherry-pick)(\s|$)') {
        # The whole command counts: a heredoc message continues past the git line.
        if ($cmd -match $mention) { Block "commit / tag messages must not mention Claude, Anthropic or a Co-Authored-By trailer." }

        foreach ($f in [regex]::Matches($rest, '(?i)(?:-F|--file(?:=|\s))\s*[''"]?(?<path>[^\s''"]+)')) {
            $path = $f.Groups['path'].Value
            if (-not [System.IO.Path]::IsPathRooted($path) -and $cwd) { $path = Join-Path $cwd $path }
            if ((Test-Path -LiteralPath $path) -and ((Get-Content -LiteralPath $path -Raw) -match $mention)) {
                Block "the commit message file mentions Claude, Anthropic or a Co-Authored-By trailer."
            }
        }
    }
}
exit 0