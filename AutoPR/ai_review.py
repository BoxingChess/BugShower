"""
AI Code Review Script
PR diff를 수집하고 Claude API로 코드 리뷰 후 GitHub에 코멘트를 등록합니다.

bug_level output:
  none   → 버그 없음 (스타일 이슈도 없음)
  style  → 스타일 이슈만 존재 (🔴🟡 없음)
  medium → 🟡 중간 이상 버그 존재 → REQUEST_CHANGES
  high   → 🔴 높음 버그 존재 → REQUEST_CHANGES
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
BASE_SHA          = os.environ["BASE_SHA"]
HEAD_SHA          = os.environ["HEAD_SHA"]

EXCLUDE_PATTERNS = [
    ".lock", ".min.js", ".min.css",
    "package-lock.json", "yarn.lock",
    "*.generated.*", "dist/", "build/",
]
MAX_DIFF_CHARS = 12_000

GITHUB_HEADERS = {
    "Authorization": f"Bearer {GITHUB_TOKEN}",
    "Accept": "application/vnd.github+json",
    "X-GitHub-Api-Version": "2022-11-28",
}

# ── diff 수집 ─────────────────────────────────────────────────────────────────

def get_pr_diff() -> str:
    result = subprocess.run(
        ["git", "diff", f"{BASE_SHA}...{HEAD_SHA}", "--unified=5"],
        capture_output=True, text=True, check=True,
    )
    return result.stdout


def filter_diff(raw_diff: str) -> str:
    sections = raw_diff.split("\ndiff --git ")
    kept = [
        s for s in sections
        if s.strip() and not any(p in s for p in EXCLUDE_PATTERNS)
    ]
    filtered = "\ndiff --git ".join(kept)
    if len(filtered) > MAX_DIFF_CHARS:
        filtered = filtered[:MAX_DIFF_CHARS] + "\n\n[... 이하 생략 (크기 초과) ...]"
    return filtered


# ── GitHub Review API ─────────────────────────────────────────────────────────

def post_pr_review(body: str, event: str) -> None:
    url = f"https://api.github.com/repos/{REPO}/pulls/{PR_NUMBER}/reviews"
    resp = requests.post(url, headers=GITHUB_HEADERS, json={"body": body, "event": event})
    resp.raise_for_status()
    print(f"리뷰 등록 완료 ({event}): {resp.json().get('html_url', '')}")


# ── Claude 리뷰 ───────────────────────────────────────────────────────────────

SYSTEM_PROMPT = """당신은 시니어 소프트웨어 엔지니어입니다.
주어진 Git diff를 분석하여 다음 두 가지 관점에서 코드 리뷰를 수행하세요.

## 리뷰 범위
1. **버그 / 오류 탐지**
   - 런타임 에러, 로직 버그, 엣지케이스 누락
   - 잠재적 NPE, 타입 불일치, 무한루프 등

2. **코드 스타일 / 컨벤션**
   - 네이밍 일관성, 함수 길이, 중복 코드
   - 주석 품질, 불필요한 임포트

## 응답 형식 (반드시 지켜주세요)
마크다운으로 작성하고, 아래 구조를 따르세요.

### 🤖 AI 코드 리뷰 결과

#### 🐛 버그 / 오류
| 파일 | 라인 | 심각도 | 내용 |
|------|------|--------|------|
| ... | ... | 🔴높음 / 🟡중간 / 🟢낮음 | ... |

(문제 없으면 "✅ 발견된 버그 없음" 으로 표기)

#### 🎨 코드 스타일 / 컨벤션
| 파일 | 내용 | 권장 수정 |
|------|------|----------|
| ... | ... | ... |

(문제 없으면 "✅ 스타일 이슈 없음" 으로 표기)

#### 💡 종합 의견
전체적인 코드 품질에 대한 2~3문장 요약.

---
> 🤖 이 리뷰는 Claude AI가 자동 생성했습니다. 최종 승인은 담당 리뷰어가 결정합니다.
"""


def call_claude(diff: str) -> str:
    client = anthropic.Anthropic(api_key=ANTHROPIC_API_KEY)
    message = client.messages.create(
        model="claude-opus-4-5",
        max_tokens=2048,
        system=SYSTEM_PROMPT,
        messages=[{
            "role": "user",
            "content": f"다음 PR의 Git diff를 리뷰해주세요:\n\n```diff\n{diff}\n```",
        }],
    )
    return message.content[0].text


def detect_bug_level(review_body: str) -> str:
    """
    리뷰 결과에서 버그 심각도를 판별합니다.
      high   → 🔴 존재
      medium → 🟡 존재 (🔴 없음)
      style  → 🔴🟡 없지만 스타일 이슈 존재
      none   → 아무 이슈 없음
    """
    if "🔴" in review_body:
        return "high"
    if "🟡" in review_body:
        return "medium"
    if "✅ 스타일 이슈 없음" in review_body:
        return "none"
    return "style"


def set_github_output(key: str, value: str) -> None:
    github_output = os.environ.get("GITHUB_OUTPUT", "")
    if github_output:
        with open(github_output, "a") as f:
            f.write(f"{key}={value}\n")
    print(f"[output] {key}={value}")


# ── 메인 ─────────────────────────────────────────────────────────────────────

def post_manual_review_comment(reason: str) -> None:
    """API 오류 시 수동 리뷰 요청 코멘트 등록"""
    body = (
        "## ⚠️ AI 리뷰 불가 — 수동 리뷰 필요\n\n"
        f"AI 코드 리뷰를 진행할 수 없습니다.\n\n"
        f"**사유**: {reason}\n\n"
        "담당자가 직접 코드를 검토한 후 Approve해 주세요.\n\n"
        "> 🤖 Claude AI"
    )
    url = f"https://api.github.com/repos/{REPO}/issues/{PR_NUMBER}/comments"
    requests.post(url, headers=GITHUB_HEADERS, json={"body": body}).raise_for_status()


def main():
    print("1/3 PR diff 수집 중...")
    raw_diff = get_pr_diff()
    if not raw_diff.strip():
        print("변경사항 없음. 리뷰를 건너뜁니다.")
        set_github_output("bug_level", "none")
        sys.exit(0)

    diff = filter_diff(raw_diff)
    print(f"    → {len(diff)} 자 분량의 diff 준비 완료")

    print("2/3 Claude API 코드 리뷰 중...")
    try:
        review_body = call_claude(diff)
        print("    → 리뷰 생성 완료")
    except anthropic.BadRequestError as e:
        if "credit balance is too low" in str(e):
            print("⚠️  크레딧 부족 → 수동 리뷰로 전환")
            post_manual_review_comment("Anthropic API 크레딧 부족 — console.anthropic.com에서 충전 필요")
            set_github_output("bug_level", "none")
            sys.exit(0)
        raise
    except anthropic.APIStatusError as e:
        print(f"⚠️  API 오류 ({e.status_code}) → 수동 리뷰로 전환")
        post_manual_review_comment(f"API 오류 발생 (status {e.status_code})")
        set_github_output("bug_level", "none")
        sys.exit(0)
    except Exception as e:
        print(f"⚠️  예상치 못한 오류 → 수동 리뷰로 전환: {e}")
        post_manual_review_comment(f"예상치 못한 오류: {type(e).__name__}")
        set_github_output("bug_level", "none")
        sys.exit(0)

    bug_level = detect_bug_level(review_body)
    set_github_output("bug_level", bug_level)

    print("3/3 GitHub PR에 리뷰 등록 중...")
    if bug_level in ("high", "medium"):
        reject_header = (
            "## ❌ AI 리뷰: 변경 요청\n\n"
            f"{'🔴 높음' if bug_level == 'high' else '🟡 중간'} 심각도 버그가 발견되어 "
            "**Merge가 자동으로 차단**되었습니다.\n"
            "아래 리뷰 내용을 확인하고 버그를 수정한 후 다시 Push해 주세요.\n\n"
            "---\n\n"
        )
        post_pr_review(reject_header + review_body, event="REQUEST_CHANGES")
        print(f"⛔ PR 자동 거부 완료 — bug_level={bug_level}")
    else:
        post_pr_review(review_body, event="COMMENT")
        print(f"✅ 리뷰 완료 — bug_level={bug_level}")


if __name__ == "__main__":
    main()
