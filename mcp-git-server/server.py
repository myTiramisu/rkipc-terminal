"""
Git MCP Server - 自动提交代码到 GitHub
提供工具：
  - git_status: 查看当前变更状态
  - git_commit_push: 自动 add + commit + push，根据 diff 生成 commit message
"""

import subprocess
import os
from mcp.server.fastmcp import FastMCP

mcp = FastMCP("git-helper")

DEFAULT_REPO = os.environ.get("GIT_REPO_PATH", "D:/software/test/rkipc-terminal")


def run_git(args: list[str], cwd: str = DEFAULT_REPO) -> str:
    result = subprocess.run(
        ["git"] + args,
        cwd=cwd,
        capture_output=True,
        text=True,
        encoding="utf-8",
    )
    if result.returncode != 0:
        raise RuntimeError(f"git {' '.join(args)} 失败:\n{result.stderr}")
    return result.stdout.strip()


def generate_commit_message(diff: str, files: str) -> str:
    """根据 diff 内容生成 commit message"""
    changed_files = [f.strip() for f in files.splitlines() if f.strip()]

    # 分析变更类型
    additions = diff.count("\n+") - diff.count("\n+++")
    deletions = diff.count("\n-") - diff.count("\n---")

    # 提取变更的目录/模块
    modules = set()
    for f in changed_files:
        parts = f.split("/")
        if len(parts) > 1:
            modules.add(parts[0] if parts[0] != "code" else parts[1] if len(parts) > 2 else parts[0])
        else:
            modules.add(os.path.splitext(f)[0])

    module_str = ",".join(sorted(modules)[:3])
    if len(modules) > 3:
        module_str += "等"

    # 判断是新增、修改还是删除
    if additions > 0 and deletions == 0:
        action = "添加"
    elif deletions > 0 and additions == 0:
        action = "删除"
    else:
        action = "更新"

    from datetime import datetime
    date_str = datetime.now().strftime("%m%d")

    return f"{date_str}#{action} {module_str}"


@mcp.tool()
def git_status(repo_path: str = "") -> str:
    """查看 git 仓库当前的变更状态，包括已修改、新增、删除的文件列表。

    Args:
        repo_path: 仓库路径，留空则使用默认路径
    """
    cwd = repo_path or DEFAULT_REPO
    status = run_git(["status", "--short"], cwd)
    if not status:
        return "工作区干净，没有待提交的变更。"
    branch = run_git(["branch", "--show-current"], cwd)
    return f"当前分支: {branch}\n\n变更文件:\n{status}"


@mcp.tool()
def git_commit_push(message: str = "", repo_path: str = "") -> str:
    """自动执行 git add + commit + push。如果不提供 message，会根据 diff 自动生成。

    Args:
        message: 自定义 commit message，留空则自动生成
        repo_path: 仓库路径，留空则使用默认路径
    """
    cwd = repo_path or DEFAULT_REPO

    # 检查是否有变更
    status = run_git(["status", "--short"], cwd)
    if not status:
        return "没有需要提交的变更。"

    # git add -A
    run_git(["add", "-A"], cwd)

    # 获取 diff 用于生成 message
    if not message:
        diff = run_git(["diff", "--cached"], cwd)
        files = run_git(["diff", "--cached", "--name-only"], cwd)
        message = generate_commit_message(diff, files)

    # git commit
    run_git(["commit", "-m", message], cwd)

    # git push
    branch = run_git(["branch", "--show-current"], cwd)
    try:
        run_git(["push", "origin", branch], cwd)
    except RuntimeError:
        # 如果远程分支不存在，设置上游
        run_git(["push", "-u", "origin", branch], cwd)

    return f"提交成功！\n分支: {branch}\nCommit: {message}\n已推送到 origin/{branch}"


if __name__ == "__main__":
    mcp.run(transport="stdio")
