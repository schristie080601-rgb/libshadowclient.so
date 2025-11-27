Project: shadowclient — native Android module

What is included:
- Android native sources under `app/src/main/cpp/`
- `CMakeLists.txt` to build `shadowclient` -> produces `libshadowclient.so`
- GitHub Actions workflow `.github/workflows/build-native.yml` to build the .so for ABIs and upload artifacts
- `build.ps1` for local Windows PowerShell builds using Android NDK/CMake

How to add these files:
1. On GitHub, open your repository.
2. Use "Add file" → "Create new file".
3. Enter the path (for example `.github/workflows/build-native.yml`), paste the content above, and commit.
4. Repeat for `build.ps1`, `.gitignore`, and `README.md`.

After committing `build-native.yml` on `main`, the workflow will run on pushes to `main` and upload `libshadowclient.so` artifacts.
