# Function to clone or update a repository
function Clone-OrUpdateRepo {
    param (
        [string]$repoUrl
    )

    $folderName = $repoUrl.Split("/")[-1].Replace(".git", "")

    if (Test-Path $folderName) {
        Write-Host "Repository '$folderName' already exists. Pulling latest changes..."
        Set-Location $folderName
        git pull
        Set-Location ..
    } else {
        Write-Host "Cloning repository '$folderName'..."
        git clone $repoUrl
    }
}

# Repositories to clone or update
$rapidjsonRepo = "https://github.com/Tencent/rapidjson.git"
$sdl2Repo = "https://github.com/libsdl-org/SDL.git"
$sdlTtfRepo = "https://github.com/libsdl-org/SDL_ttf.git"
$sdlImageRepo = "https://github.com/libsdl-org/SDL_image.git"

# List of repositories
$repos = @(
    $rapidjsonRepo,
    $sdl2Repo,
    $sdlTtfRepo,
    $sdlImageRepo
)

# Loop through each repository and clone or update
foreach ($repo in $repos) {
    Clone-OrUpdateRepo -repoUrl $repo
}

Write-Host "All repositories are up to date!"
