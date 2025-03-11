import sys
import os
import json


def load_config():
    with open("scripts/burner_release_config.json", "r") as f:
        return json.load(f)


def get_project_version():
    with open("CMakeLists.txt") as f:
        for line in f:
            if line.startswith("set(PROJECT_VER"):
                return line.split("\"")[1].split("\"")[0].strip()
    return None


def merge_bin():
    if os.system("idf.py merge-bin") != 0:
        print("merge bin failed")
        sys.exit(1)


def copy_bin(bin_name, project_version):
    if not os.path.exists("releases"):
        os.makedirs("releases")
    output_path = f"releases/{bin_name}_V{project_version}_0x0.bin"
    if os.path.exists(output_path):
        os.remove(output_path)
    if os.system(f"cp build/merged-binary.bin {output_path}") != 0:
        print("copy bin failed")
        sys.exit(1)
    print(f"copy bin to {output_path} done")


def release(target_info):
    board_type = target_info["board_type"]
    board_config = target_info["board_config"]
    target = target_info["target"]
    builds = target_info["builds"]
    bin_name = target_info["binName"]

    project_version = get_project_version()
    print(f"Project Version: {project_version}")
    release_path = f"releases/{bin_name}_V{project_version}_0x0.bin"
    if os.path.exists(release_path):
        print(f"跳过 {board_type} 因为 {release_path} 已存在")
        return

    for build in builds:
        name = build["name"]
        if not name.startswith(board_type):
            raise ValueError(f"name {name} 必须 {board_type} 开头")

        sdkconfig_append = [f"{board_config}=y"] + \
            build.get("sdkconfig_append", [])

        print(f"name: {name}")
        print(f"target: {target}")
        for append in sdkconfig_append:
            print(f"sdkconfig_append: {append}")

        os.environ.pop("IDF_TARGET", None)
        if os.system(f"idf.py set-target {target}") != 0:
            print("set-target failed")
            sys.exit(1)

        with open("sdkconfig", "a") as f:
            f.write("\n" + "\n".join(sdkconfig_append) + "\n")

        if os.system(f"idf.py -DBOARD_NAME={name} build") != 0:
            print("build failed")
            sys.exit(1)

        merge_bin()
        copy_bin(bin_name, project_version)
        print("-" * 80)


if __name__ == "__main__":
    config = load_config()
    found = False
    for target in config["targets"]:
        if sys.argv[1] == 'all' or target["board_type"] == sys.argv[1]:
            release(target)
            found = True

    if not found:
        print(f"未找到板子类型: {sys.argv[1]}")
        print("可用的板子类型:")
        for target in config["targets"]:
            print(f"  {target['board_type']}")