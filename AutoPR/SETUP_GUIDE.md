# GitHub Branch Protection 설정 가이드

담당자의 최종 승인을 필수화하는 Branch Protection 설정 방법입니다.

---

## 1. Repository Settings → Branches

`Settings` → `Branches` → `Add branch ruleset` 클릭

## 2. 권장 설정값

| 항목 | 설정값 | 설명 |
|------|--------|------|
| Branch name pattern | `main` (또는 `master`) | 보호할 브랜치 |
| Require a pull request before merging | ✅ ON | PR 없이 직접 push 금지 |
| Required number of approvals | `1` 이상 | 담당자 승인 필수 |
| Dismiss stale reviews on new commits | ✅ ON | 새 커밋 시 기존 승인 초기화 |
| Require status checks to pass | ✅ ON | AI 리뷰 workflow 완료 필수 |
| Status check name | `AI Code Review / Claude AI Code Review` | workflow job 이름 |
| Do not allow bypassing | ✅ ON | 관리자도 규칙 적용 |

---

## 3. Secrets 등록

`Settings` → `Secrets and variables` → `Actions` → `New repository secret`

| Secret 이름 | 값 |
|-------------|----|
| `ANTHROPIC_API_KEY` | Anthropic Console에서 발급한 API 키 |

> `GITHUB_TOKEN`은 GitHub Actions가 자동 제공하므로 별도 등록 불필요

---

## 4. 전체 흐름 요약

```
개발자 PR 생성
   ↓
GitHub Actions 자동 실행 (ai-code-review.yml)
   ↓
Claude AI 리뷰 코멘트 PR에 자동 등록
   ↓
담당자가 리뷰 확인 후 Approve
   ↓
Merge 가능 상태로 전환
```

---

## 5. 폴더 구조

```
your-repo/
├── .github/
│   └── workflows/
│       └── ai-code-review.yml   ← GitHub Actions 워크플로우
└── scripts/
    └── ai_review.py             ← Claude 리뷰 스크립트
```
