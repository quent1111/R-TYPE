# GitHub Actions CI/CD

Automated workflows for R-TYPE project quality assurance and deployment.

## 🔄 Active Workflows

### 1. **CI/CD Pipeline** (`ci.yml`)
**Triggers:** Push to `main`, `Architecture`, `develop`

**Stages:**
1. ✅ Code Quality (clang-format + clang-tidy)
2. ✅ Build & Test (Ubuntu with all dependencies)
3. ✅ Multi-platform Build (Linux, macOS, Windows)
4. 🚀 **Deploy to Epitech** (main branch only, requires token)
5. 📊 Summary Report

### 2. **Pull Request Checks** (`pr-check.yml`)
**Triggers:** Pull requests to `main`

**Actions:**
- ✅ Format validation
- ✅ Build verification
- ✅ Test execution
- 💬 Auto-comments on PR with results

### 3. **Code Coverage** (`coverage.yml`)
**Triggers:** Push to `main`/`develop`, Pull requests

**Outputs:**
- 📊 Coverage report (HTML)
- 📤 Upload to Codecov
- 📦 Artifacts for 30 days

## 🔑 Setup Required

### Epitech Deployment Token

To enable automatic deployment to the Epitech repository:

1. **Create a GitHub Personal Access Token:**
   - Go to: Settings → Developer Settings → Personal Access Tokens → Tokens (classic)
   - Scopes needed: `repo`, `workflow`

2. **Add to Repository Secrets:**
   - Repository Settings → Secrets → Actions
   - Name: `EPITECH_DEPLOY_TOKEN`
   - Value: Your token

3. **Deployment will automatically happen when:**
   - ✅ All tests pass on `main` branch
   - ✅ Build succeeds on all platforms
   - ✅ Code quality checks pass

**Target repository:** `https://github.com/EpitechPGE3-2025/G-CPP-500-NAN-5-2-rtype-4`

## 📋 Status Badges

Add these to your README.md:

```markdown
![CI/CD](https://github.com/quent1111/R-TYPE/actions/workflows/ci.yml/badge.svg)
![Coverage](https://github.com/quent1111/R-TYPE/actions/workflows/coverage.yml/badge.svg)
```

## 🛠️ Local Testing

Before pushing, test locally:

```bash
# Format check
./scripts/format.sh

# Format and fix
./scripts/format.sh --format

# Build and test
./r-type.sh test

# Coverage report
./r-type.sh coverage
```

## 📚 Documentation

- **[CI_SETUP.md](CI_SETUP.md)** - Detailed setup guide
- **[Workflows README](workflows/)** - Individual workflow docs

## 🚨 Troubleshooting

### Build Fails in CI but Works Locally

1. Check dependencies are installed
2. Verify Conan cache is clean
3. Look for platform-specific code

### Deployment Doesn't Trigger

1. Verify you're on `main` branch
2. Check `EPITECH_DEPLOY_TOKEN` is set
3. Ensure all previous jobs passed
4. Review job logs in Actions tab

### Format Check Fails

```bash
# Fix locally
./scripts/format.sh --format

# Commit and push
git add .
git commit -m "fix: apply code formatting"
git push
```

## 💡 Best Practices

1. **Always create Pull Requests** - Never push directly to main
2. **Wait for CI** - Green checks before merging
3. **Fix formatting locally** - Run `./scripts/format.sh --format`
4. **Add tests** - Coverage should increase, not decrease
5. **Monitor Actions** - Check the Actions tab regularly

## 📈 Coverage Goals

| Component | Target |
|-----------|--------|
| ECS Core | 90%+ |
| Network | 85%+ |
| Game Logic | 80%+ |
| Rendering | 70%+ |

---

**Need help?** Check the [CI_SETUP.md](CI_SETUP.md) guide or GitHub Actions logs.
