import contextlib
import io
import json
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

import nebs_test_registration as lint


class RegistrationTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        (self.root / "Tools").mkdir()
        (self.root / "TaskFiles/Tests").mkdir(parents=True)

    def write(self, name, content):
        path = self.root / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(content, encoding="utf-8")
        return path

    def config(self, entries):
        self.write("Tools/tests.jsonc", json.dumps({"tests": entries}) + " // suite")

    def test_inline_external_and_quoted_paths(self):
        self.write("TaskFiles/Tests/first.nebs", "")
        self.write("TaskFiles/Tests/with space.nebs", "")
        self.write("TaskFiles/Tests/unregistered.nebs", "")
        self.write("TaskFiles/Tests/helper.nebs", "")
        self.write(
            "Tools/manifest.json",
            json.dumps([{"command": 'task "TaskFiles/Tests/with space.nebs"'}]),
        )
        self.config([
            {"command": "task ./TaskFiles/Tests/first.nebs"},
            "Tools/manifest.json",
            {"command": "echo TaskFiles/Tests/unregistered.nebs"},
            {"command": "task-exec TaskFiles/Tests/helper.nebs"},
        ])
        self.assertEqual(lint.find_unregistered(self.root), [
            Path("TaskFiles/Tests/helper.nebs"),
            Path("TaskFiles/Tests/unregistered.nebs"),
        ])

    def test_unreferenced_manifest_does_not_register_test(self):
        self.write("TaskFiles/Tests/example.nebs", "")
        self.write("Tools/unused.json", '[{"command": "task TaskFiles/Tests/example.nebs"}]')
        self.config([])
        self.assertEqual(lint.find_unregistered(self.root), [Path("TaskFiles/Tests/example.nebs")])

    def test_suppression_requires_a_standalone_comment(self):
        self.write("TaskFiles/Tests/fixture.nebs", "#NOLINT(test-registration) used as a fixture\n")
        self.write("TaskFiles/Tests/indented.nebs", "  #NOLINT(test-registration)\n")
        self.write("TaskFiles/Tests/other.nebs", "#NOLINT(other)\n")
        self.write("TaskFiles/Tests/string.nebs", 'echo "#NOLINT(test-registration)"\n')
        self.write("TaskFiles/Tests/embedded.nebs", "# note #NOLINT(test-registration)\n")
        self.write("TaskFiles/Tests/suffix.nebs", "#NOLINT(test-registration)extra\n")
        self.config([])
        self.assertEqual(lint.find_unregistered(self.root), [
            Path("TaskFiles/Tests/embedded.nebs"),
            Path("TaskFiles/Tests/other.nebs"),
            Path("TaskFiles/Tests/string.nebs"),
            Path("TaskFiles/Tests/suffix.nebs"),
        ])

    def test_unreadable_task_is_reported(self):
        path = self.root / "TaskFiles/Tests/invalid.nebs"
        path.write_bytes(b"\xff")
        self.config([])
        with self.assertRaisesRegex(ValueError, "invalid.nebs"):
            lint.find_unregistered(self.root)

    def test_missing_files_print_suppression_tip(self):
        self.write("TaskFiles/Tests/missing.nebs", "")
        self.config([])
        output = io.StringIO()
        with patch.object(lint, "ROOT", self.root), contextlib.redirect_stdout(output):
            self.assertEqual(lint.main(), 1)
        self.assertIn("TaskFiles/Tests/missing.nebs", output.getvalue())
        self.assertIn("#NOLINT(test-registration)", output.getvalue())

    def test_invalid_configuration_is_reported(self):
        for entries, manifest_content in [
            (["Tools/missing.json"], None),
            (["Tools/manifest.json"], "{"),
            (["Tools/manifest.json"], "{}"),
            ([{"command": ["task example.nebs"]}], None),
            ([{"command": "task 'unfinished"}], None),
            ([{"command": "task"}], None),
        ]:
            with self.subTest(entries=entries, manifest=manifest_content):
                if manifest_content is not None:
                    self.write("Tools/manifest.json", manifest_content)
                self.config(entries)
                with self.assertRaises(ValueError):
                    lint.find_unregistered(self.root)

    def test_missing_tests_directory_is_an_error(self):
        with tempfile.TemporaryDirectory() as root:
            with self.assertRaisesRegex(ValueError, "test directory does not exist"):
                lint.find_unregistered(Path(root))


if __name__ == "__main__":
    unittest.main()
