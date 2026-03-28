"""
AI PR Description Generator
커밋 메시지 + diff를 분석하여 표준화된 PR 설명을 자동 생성합니다.
PR body가 비어있을 때만 실행됩니다.
"""

import os
import sys
import subprocess
import requests
import anthropic

# ── 환경변수 ─────────────────────────────────────────────────────────────────
ANTHROPIC_API_KEY = os.environ["ANTHROPIC_API_KEY"]
GITHUB_TOKEN      = os.environ["GITHUB_TOKEN"]
REPO              = os.environ["REPO"]
PR_NUMBER         = os.environ["PR_NUMBER"]
PR_TITLE          = os.environ.get("PR_TITLE", "")
BASE_SHA          = os.environ["BASE_SHA"]
HEAD_SHA          = os.environ["HEAD_SHA"]
BASE_BRANCH       = os.environ.get("BASE_BRANCH", "main")

MAX_DIFF_CHARS = 10_000

GITHUB_HEADERS = {
    "Authorization": f"Bearer {GITHUB_TOKEN}",
    "Accept": "application/vnd.github+json",
    "X-GitHub-Api-Version": "2022-11-28",
}

# ── 데이터 수집 ──────────────────────────────────────────────────────────────

def get_commit_messages() -> str:
    result = subprocess.run(
        ["git", "log", f"{BASE_SHA}..{HEAD_SHA}", "--pretty=format:- %s%n%b"],
        capture_output=True, text=True, check=True,
        encoding="utf-8", errors="replace",
    )
    return result.stdout.strip()


def get_changed_files() -> list[str]:
    result = subprocess.run(
        ["git", "diff", "--name-only", f"{BASE_SHA}...{HEAD_SHA}"],
        capture_output=True, text=True, check=True,
        encoding="utf-8", errors="replace",
    )
    return [f for f in result.stdout.strip().splitlines() if f]


def get_diff() -> str:
    result = subprocess.run(
        ["git", "diff", f"{BASE_SHA}...{HEAD_SHA}", "--unified=3",
         "--diff-filter=ACMRT"],
        capture_output=True, text=True, check=True,
        encoding="utf-8", errors="replace",
    )
    diff = result.stdout
    if len(diff) > MAX_DIFF_CHARS:
        diff = diff[:MAX_DIFF_CHARS] + "\n\n[... 생략 ...]"
    return diff


# ── Claude 호출 ──────────────────────────────────────────────────────────────

SYSTEM_PROMPT = """당신은 PR 설명 작성을 돕는 시니어 개발자입니다.
주어진 정보(PR 제목, 커밋 메시지, 변경 파일 목록, Git diff)를 분석하여
팀 전체가 일관된 형식으로 PR을 작성할 수 있도록 표준화된 설명을 생성하세요.

반드시 아래 마크다운 형식을 그대로 사용하세요. 헤더 이모지와 섹션 제목을 바꾸지 마세요.
내용이 불분명한 경우 diff에서 추론하고, 추론이 어려우면 `_작성 필요_` 라고 표기하세요.

---

## 📌 변경 목적 / 배경
> 이 PR이 왜 필요한지 서술합니다.

(내용)

## 📝 주요 변경사항
> 무엇을 어떻게 바꿨는지 핵심만 간결하게 서술합니다.

- (변경사항 1)
- (변경사항 2)

## 📁 변경 파일 요약
| 파일 | 변경 유형 | 설명 |
|------|----------|------|
| (파일명) | 추가 / 수정 / 삭제 | (한 줄 설명) |

---
> 🤖 이 PR 설명은 Claude AI가 자동 생성했습니다. 내용을 검토한 후 필요하면 수정해 주세요.
"""


def generate_description(title: str, commits: str, files: list[str], diff: str) -> str:
    client = anthropic.Anthropic(api_key=ANTHROPIC_API_KEY)
    user_content = f"""PR 제목: {title}

커밋 메시지:
{commits if commits else "(없음)"}

변경된 파일 ({len(files)}개):
{chr(10).join(f"- {f}" for f in files[:30])}
{"..." if len(files) > 30 else ""}

Git diff:
```diff
{diff}
```
"""
    message = client.messages.create(
        model="claude-opus-4-5",
        max_tokens=1500,
        system=SYSTEM_PROMPT,
        messages=[{"role": "user", "content": user_content}],
    )
    return message.content[0].text


# ── GitHub PR body 업데이트 ──────────────────────────────────────────────────

def update_pr_body(body: str) -> None:
    url = f"https://api.github.com/repos/{REPO}/pulls/{PR_NUMBER}"
    resp = requests.patch(url, headers=GITHUB_HEADERS, json={"body": body})
    resp.raise_for_status()
    print(f"PR 설명 업데이트 완료: {resp.json().get('html_url', '')}")


# ── 메인 ─────────────────────────────────────────────────────────────────────

def main():
    print("1/4 커밋 메시지 수집 중...")
    commits = get_commit_messages()
    print(f"    → {len(commits.splitlines())}개 커밋")

    print("2/4 변경 파일 목록 수집 중...")
    files = get_changed_files()
    print(f"    → {len(files)}개 파일")

    print("3/4 diff 수집 중...")
    diff = get_diff()
    print(f"    → {len(diff)} 자")

    print("4/4 Claude로 PR 설명 생성 중...")
    try:
        description = generate_description(PR_TITLE, commits, files, diff)
        print("    → 생성 완료")
        update_pr_body(description)
        print("✅ PR 설명 자동 등록 완료!")
    except anthropic.BadRequestError as e:
        err_msg = str(e)
        print(f"⚠️  BadRequestError 상세: {err_msg}")
        sys.exit(0)
    except anthropic.APIStatusError as e:
        print(f"⚠️  APIStatusError 상세: status={e.status_code} body={str(e)}")
        sys.exit(0)
    except Exception as e:
        import traceback
        print(f"⚠️  예상치 못한 오류 상세:\n{traceback.format_exc()}")
        sys.exit(0)


if __name__ == "__main__":
    main()
