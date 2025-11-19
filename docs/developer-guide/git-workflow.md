# Git Workflow

## Branch Strategy

- `main` - Stable production code
- `develop` - Development branch
- `feature/*` - Feature branches
- `bugfix/*` - Bug fix branches
- `hotfix/*` - Urgent fixes

## Commit Messages

Follow conventional commits:

```
type(scope): subject

body

footer
```

**Types**: feat, fix, docs, style, refactor, test, chore

**Example**:
```
feat(ecs): add component query system

Implement view() function to query entities with specific components.

Closes #123
```

## Coming soon...

More detailed workflow documentation.
