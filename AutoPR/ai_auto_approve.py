"""
AI Auto Approve Script
PR 작성자를 제외한 나머지 담당자가 전원 REVIEWER_ABSENT_LIST에 있을 때 자동 Approve & Merge합니다.

REVIEWER_ABSENT_LIST 관리:
  GitHub Actions 탭 → "Toggle My Absence" 워크플로우 실행 → 부재 시작 / 복귀 선택

현황 확인:
  GitHub Actions 탭 → "Check Reviewer Status" 워크플로우 실행
"""

import os
import sys
import requests

GITHUB_TOKEN         = os.environ["GITHUB_TOKEN"]
REPO                 = os.environ["REPO"]
PR_NUMBER            = os.environ["PR_NUMBER"]
PR_SHA               = os.environ["PR_SHA"]
BUG_LEVEL            = os.environ.get("BUG_LEVEL", "none")

# "none"은 빈 값을 의미하므로 제외
REVIEWER_ABSENT_LIST = {
    r.strip()
    for r in os.environ.get("REVIEWER_ABSENT_LIST", "").split(",")
    if r.strip() and r.strip() != "none"
}

GITHUB_HEADERS = {
    "Authorization": f"Bearer {GITHUB_TOKEN}",
    "Accept": "application/vnd.github+json",
    "X-GitHub-Api-Version": "2022-11-28",
}


def get_pr_info() -> tuple[list[str], str]:
    """Assignee 목록과 PR 작성자 반환"""
    url = f"https://api.github.com/repos/{REPO}/pulls/{PR_NUMBER}"
    resp = requests.get(url, headers=GITHUB_HEADERS)
    resp.raise_for_status()
    data = resp.json()
    assignees = [a["login"] for a in data.get("assignees", [])]
    author = data["user"]["login"]
    return assignees, author


def post_comment(body: str) -> None:
    url = f"https://api.github.com/repos/{REPO}/issues/{PR_NUMBER}/comments"
    requests.post(url, headers=GITHUB_HEADERS, json={"body": body}).raise_for_status()


def approve_pr(absent_reviewers: list[str]) -> None:
    url = f"https://api.github.com/repos/{REPO}/pulls/{PR_NUMBER}/reviews"
    requests.post(url, headers=GITHUB_HEADERS, json={
        "body": (
            f"## ✅ AI 자동 승인\n\n"
            f"담당자({', '.join(f'@{r}' for r in absent_reviewers)}) 부재 + "
            f"🔴🟡 버그 없음 조건 충족으로 자동 승인합니다.\n\n"
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
    print(f"부재 목록: {REVIEWER_ABSENT_LIST or '(없음)'}")

    assignees, author = get_pr_info()
    print(f"PR 작성자: @{author}")
    print(f"PR 담당자: {assignees or '(없음)'}")

    if not assignees:
        print("담당자 미지정 — 수동 Approve 대기")
        sys.exit(0)

    # PR 작성자(나)를 제외한 나머지 담당자만 체크
    other_assignees = [a for a in assignees if a != author]

    if not other_assignees:
        print("나 혼자만 담당자 — 수동 Approve 대기")
        sys.exit(0)

    absent_reviewers = [a for a in other_assignees if a in REVIEWER_ABSENT_LIST]
    all_others_absent = len(absent_reviewers) == len(other_assignees)

    if not all_others_absent:
        present = [a for a in other_assignees if a not in REVIEWER_ABSENT_LIST]
        print(f"다른 담당자 재직 중({present}) — 수동 Approve 대기")
        sys.exit(0)

    print(f"나를 제외한 담당자 전원 부재({absent_reviewers}) → 자동 승인 진행")

    print("1/3 부재 알림 코멘트 등록 중...")
    post_comment(
        f"### 🔔 담당자 부재 자동 처리 안내\n\n"
        f"담당자({', '.join(f'@{r}' for r in absent_reviewers)})가 부재 중이고\n"
        f"AI 리뷰 결과 **🔴🟡 버그 없음**으로 자동 Approve & Merge를 진행합니다.\n\n"
        f"- bug_level: `{BUG_LEVEL}`\n\n"
        f"담당자 복귀 후 이 PR을 사후 검토해 주세요.\n\n"
        f"> 🤖 자동 처리 by Claude AI"
    )

    print("2/3 Approve 중...")
    approve_pr(absent_reviewers)

    print("3/3 Merge 중...")
    merge_pr()

    print("✅ 자동 승인 & Merge 완료!")


if __name__ == "__main__":
    main()
