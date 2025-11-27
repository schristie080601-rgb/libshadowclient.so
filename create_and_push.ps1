param(
    [string]$RepoName = "shadowclient",
    [string]$Visibility = "public"  # "public" or "private"
)

# Preconditions: git installed. If you want to use gh, install GitHub CLI and run 'gh auth login' first.
if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
    Write-Error "git is not installed or not on PATH."
    exit 1
}

# Initialize git if needed
if (-not (Test-Path .git)) {
    git init
}

git add --all
git commit -m "Initial commit" 2>$null

# ensure main branch
git branch -M main

$gh = Get-Command gh -ErrorAction SilentlyContinue
if ($gh) {
    # Create repo and push (non-interactive)
    gh repo create $RepoName --$Visibility --source . --remote origin --push --confirm
    if ($LASTEXITCODE -ne 0) { Write-Error "gh repo create failed"; exit 1 }
    Write-Host "Repository created and pushed to GitHub: $RepoName"
} else {
    Write-Host "gh CLI not found. Create an empty repo on GitHub named '$RepoName', then run these commands:"
    Write-Host "`n    git remote add origin https://github.com/<your-username>/$RepoName.git"
    Write-Host "    git push -u origin main`n"
}
