"""Post Discord notifications for CI events.

Usage:
    python notify_discord.py new-build    # New game image published
    python notify_discord.py new-pr       # New pull request opened

Subcommand environment variables (set in workflow step):

  new-build:
    DISCORD_NEW_BUILD_WEBHOOK_URL  - Discord webhook endpoint
    DISCORD_ADMIN_ROLE_ID          - Discord role ID to mention (optional)
    GH_TOKEN                       - GitHub token (for gh CLI auth)

  new-pr:
    DISCORD_PR_WEBHOOK_URL         - Discord webhook endpoint

Environment variables (provided automatically by GitHub Actions runner):
    GITHUB_REPOSITORY              - owner/repo (e.g. sneezymud/sneezymud)
    GITHUB_SHA                     - commit SHA that triggered the build
    GITHUB_EVENT_PATH              - path to JSON file with webhook event payload
"""

import json
import os
import subprocess
import sys
import urllib.request


def send(webhook_url, content, mention_role_id=""):
    """Send a message to a Discord webhook."""
    req = urllib.request.Request(
        webhook_url,
        data=json.dumps({
            "content": content,
            "allowed_mentions": {"roles": [mention_role_id]} if mention_role_id else {},
        }).encode(),
        headers={"Content-Type": "application/json", "User-Agent": "SneezyMUD-CI"},
    )
    urllib.request.urlopen(req, timeout=10)


def new_build():
    """Notify that a new game image is available."""
    repo = os.environ["GITHUB_REPOSITORY"]
    sha = os.environ["GITHUB_SHA"]
    webhook_url = os.environ["DISCORD_NEW_BUILD_WEBHOOK_URL"]
    admin_role_id = os.environ.get("DISCORD_ADMIN_ROLE_ID", "")

    mention = f"<@&{admin_role_id}> " if admin_role_id else ""
    msg = f"{mention}New game image available! Reboot to apply the update."
    try:
        result = subprocess.run(
            ["gh", "api", f"repos/{repo}/commits/{sha}/pulls", "--jq", ".[0] // empty"],
            capture_output=True, text=True, check=True,
        )
        if result.stdout.strip():
            pr = json.loads(result.stdout)
            msg += f"\nSummary: {pr['title']} (<{pr['html_url']}>)"
    except (subprocess.CalledProcessError, json.JSONDecodeError, KeyError) as e:
        print(f"Warning: could not fetch PR details: {e}", file=sys.stderr)

    send(webhook_url, msg, admin_role_id)


def new_pr():
    """Notify that a new pull request was opened."""
    webhook_url = os.environ["DISCORD_PR_WEBHOOK_URL"]

    with open(os.environ["GITHUB_EVENT_PATH"]) as f:
        pr = json.load(f)["pull_request"]

    author = pr["user"]["login"]
    msg = f"New PR from **{author}**: {pr['title']}\n<{pr['html_url']}>"

    send(webhook_url, msg)


commands = {
    "new-build": new_build,
    "new-pr": new_pr,
}

if __name__ == "__main__":
    if len(sys.argv) != 2 or sys.argv[1] not in commands:
        print(f"Usage: {sys.argv[0]} <{'|'.join(commands)}>", file=sys.stderr)
        sys.exit(1)
    commands[sys.argv[1]]()
