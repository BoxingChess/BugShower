"""
AI Auto Approve Script
담당자 부재(REVIEWER_ABSENT=true) + 버그 없음 조건 충족 시
GitHub Bot이 PR을 자동으로 Approve하고 Merge합니다.

담당자 재지정은 GitHub PR 페이지에서 직접 수동으로 처리하세요.
"""

import os
import sys
import requests

GITHUB_TOKEN = os.environ["GITHUB_TOKEN"]
REPO         = os.environ["REPO"]
PR_NUMBER    = os.environ["PR_NUMBER"]
PR_SHA       = os.environ["PR_SHA"]
BUG_LEVEL    = os.environ.get("BUG_LEVEL", "none")

GITHUB_HEADERS = {
    "Authorization": f"Bearer {GITHUB_TOKEN}",
    "Accept": "application/vnd.github+json",
    "X-GitHub-Api-Version": "2022-11-28",
}

def post_comment(body: str) -> None:
    url = f"https://api.github.com/repos/{REPO}/issues/{PR_NUMBER}/comments"
    requests.post(url, headers=GITHUB_HEADERS, json={"body": body}).raise_for_status()


def approve_pr() -> None:
    url = f"https://api.github.com/repos/{REPO}/pulls/{PR_NUMBER}/reviews"
    requests.post(url, headers=GITHUB_HEADERS, json={
        "body": (
            f"## ✅ AI 자동 승인\n\n"
            f"담당자 부재(`REVIEWER_ABSENT=true`) + 🔴🟡 버그 없음 조건 충족으로 자동 승인합니다.\n\n"
            f"- bug_level: `{BUG_LEVEL}`\n\n"
            f"> 🤖 담당자 복귀 후 검토 바랍니다."
        ),
        "event": "APPROVE",
    }).raise_for_status()
    print("  ✅ Approve 완료")


def merge_pr() -> None:
    url = f"https://api.github.com/repos/{REPO}/pulls/{PR_NUMBER}/merge"
    resp = requests.put(url, headers=GITHUB_HEADERS, json={
        "commit_title": f"Auto-merge PR #{PR_NUMBER} (reviewer absent)",
        "commit_message": f"자동 승인 및 Merge — 담당자 부재 중 AI 리뷰 통과\nbug_level: {BUG_LEVEL}",
        "sha": PR_SHA,
        "merge_method": "squash",
    })
    if resp.status_code == 405:
        print("  ⚠️  Merge 불가 — Branch Protection 조건 미충족")
        sys.exit(1)
    if resp.status_code == 409:
        print("  ⚠️  Merge 충돌 — 작업자가 직접 해결 필요")
        sys.exit(1)
    resp.raise_for_status()
    print("  ✅ Merge 완료")


def main():
    print(f"담당자 부재 자동 승인 시작 — bug_level={BUG_LEVEL}")

    print("1/3 부재 알림 코멘트 등록 중...")
    post_comment(
        f"### 🔔 담당자 부재 자동 처리 안내\n\n"
        f"`REVIEWER_ABSENT=true` 설정 + AI 리뷰 결과 **🔴🟡 버그 없음**으로 자동 Approve & Merge를 진행합니다.\n\n"
        f"- bug_level: `{BUG_LEVEL}`\n\n"
        f"담당자 복귀 후 이 PR을 사후 검토해 주세요.\n\n"
        f"> 🤖 자동 처리 by Claude AI"
    )

    print("2/3 Approve 중...")
    approve_pr()

    print("3/3 Merge 중...")
    merge_pr()

    print("✅ 자동 승인 & Merge 완료!")


if __name__ == "__main__":
    main()
