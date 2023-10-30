Import("env")
import os
import subprocess
import shutil

board_config = env.BoardConfig()

if board_config.get("filesystem","littlefs"):

    # Define the URL of the Git repository you want to clone
    repo_url = "https://github.com/joltwallet/esp_littlefs.git"

    print("Cloning Git repository:", repo_url)

    # Define the local path where you want to clone the repository
    # local_repo_path = env.subst("$BUILD_DIR/yourrepository")
    local_repo_path = env.subst("$PROJECT_DIR/managed_components/esp_littlefs")

    # Check if the repository has already been cloned
    if not os.path.exists(os.path.join(local_repo_path,"idf_component.yml")):
        if os.path.exists(local_repo_path):
            shutil.rmtree(local_repo_path, ignore_errors=True)
        # Clone the Git repository
        cmd = ["git", "clone", repo_url, local_repo_path]
        result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        if result.returncode == 0:
            print("Cloned Git repository to:", local_repo_path)
        else:
            print("Failed to clone Git repository:", result.stderr)
    else:
        print("Git repository already cloned at:", local_repo_path)


    # Define the URL of the Git repository you want to clone
    repo_url = "https://github.com/espressif/arduino-esp32.git"

    print("Cloning Git repository:", repo_url)

    # Define the branch you want to checkout
    branch_name = "2.0.9"

    # Define the local path where you want to clone the repository
    local_repo_path = env.subst("$PROJECT_DIR/managed_components/arduino")

    # Check if the repository has already been cloned
    if not os.path.exists(os.path.join(local_repo_path,"idf_component.yml")):
        if os.path.exists(local_repo_path):
            shutil.rmtree(local_repo_path, ignore_errors=True)
        # Clone the Git repository and checkout the specified branch
        cmd = ["git", "clone", repo_url, local_repo_path]
        result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        
        if result.returncode == 0:
            print("Cloned Git repository to:", local_repo_path)
        else:
            print("Failed to clone Git repository:", result.stderr)
    else:
        print("Git repository already cloned at:", local_repo_path)

    # Check out the specified branch
    if os.path.exists(local_repo_path):
        os.chdir(local_repo_path)
        cmd = ["git", "checkout", branch_name]
        result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        
        if result.returncode == 0:
            print("Checked out branch:", branch_name)
        else:
            print("Failed to checkout branch:", result.stderr)

