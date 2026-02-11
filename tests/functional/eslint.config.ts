// ESLint over oxlint: type-aware rules like no-floating-promises (catches
// forgotten await on async MUD commands), no-misused-promises, and
// await-thenable are critical for an async telnet harness and are only stable
// in eslint. The speed difference is irrelevant at this project's scale.

import eslintComments from "@eslint-community/eslint-plugin-eslint-comments/configs";
import eslint from "@eslint/js";
import prettier from "eslint-config-prettier/flat";
import { importX } from "eslint-plugin-import-x";
import jest from "eslint-plugin-jest";
import { configs as perfectionist } from "eslint-plugin-perfectionist";
import { configs as regExp } from "eslint-plugin-regexp";
import unicorn from "eslint-plugin-unicorn";
import { defineConfig } from "eslint/config";
import tseslint, { type Config } from "typescript-eslint";

const config: Config = defineConfig([
  // Global ignores
  {
    ignores: ["node_modules/**", "output/**"],
  },

  // Base JS rules
  {
    extends: [eslint.configs.recommended],
    rules: {
      // Keep arrow functions as concise as possible
      "arrow-body-style": "warn",

      // Always use curly braces for blocks for safety/consistency
      curly: ["warn", "all"],

      // Prefer arrow functions for callbacks only
      "prefer-arrow-callback": ["warn"],
    },
  },

  // Import hygiene
  {
    plugins: {
      // @ts-expect-error - importX type doesn't match eslint's plugin type: https://github.com/un-ts/eslint-plugin-import-x/issues/203
      "import-x": importX,
    },
    rules: {
      // Report circular dependencies
      "import-x/no-cycle": "error",

      // Enforce named exports, makes refactoring and tracing code easier
      "import-x/no-default-export": "error",

      // Enforce good import practices
      "import-x/no-duplicates": "error",
      "import-x/no-named-as-default": "error",
      "import-x/no-named-as-default-member": "error",
    },
  },

  // Some config files require default exports
  {
    files: ["*.config.ts"],
    rules: {
      "import-x/no-default-export": "off",
    },
  },

  // Enforce consistent ordering throughout code
  {
    extends: [perfectionist["recommended-natural"]],
    rules: {
      // Sort module declarations in order of usage; makes code easier to reason about.
      "perfectionist/sort-modules": [
        "warn",
        {
          type: "usage",
        },
      ],
    },
  },

  // Unicorn (modern JS patterns)
  {
    extends: [unicorn.configs.recommended],
    rules: {
      // Null is necessary for telnet interactions
      "unicorn/no-null": "off",

      // Too opinionated
      "unicorn/prevent-abbreviations": "off",
    },
  },

  // TypeScript (strict type-aware linting)
  {
    extends: [
      tseslint.configs.strictTypeChecked,
      tseslint.configs.stylisticTypeChecked,
    ],
    files: ["**/*.ts"],
    languageOptions: {
      parserOptions: {
        projectService: true,
        tsconfigRootDir: import.meta.dirname,
      },
    },
    rules: {
      // Keeps array types more readable based on complexity
      "@typescript-eslint/array-type": ["warn", { default: "array-simple" }],

      // Allow `while (true)` — idiomatic in read-until-prompt/idle loops
      "@typescript-eslint/no-unnecessary-condition": [
        "error",
        {
          allowConstantLoopConditions: "only-allowed-literals",
        },
      ],

      // Prevent unsafe assertions that could lead to runtime errors
      "@typescript-eslint/no-unsafe-type-assertion": "error",

      // Delegate unused-var detection to eslint (richer than tsconfig's noUnusedLocals):
      // _prefix suppresses warnings for intentionally unused vars like catch bindings
      // and destructured rest siblings.
      "@typescript-eslint/no-unused-vars": [
        "warn",
        {
          args: "all",
          argsIgnorePattern: "^_",
          caughtErrors: "all",
          caughtErrorsIgnorePattern: "^_",
          destructuredArrayIgnorePattern: "^_",
          ignoreRestSiblings: true,
          varsIgnorePattern: "^_",
        },
      ],

      // Enforce explicit sorting of arrays to avoid unexpected results
      "@typescript-eslint/require-array-sort-compare": "error",

      // Allow raw numbers in template literals for convenience
      "@typescript-eslint/restrict-template-expressions": [
        "error",
        { allowNumber: true },
      ],
    },
  },

  // Jest test best practices (works with bun:test via globalPackage setting)
  {
    extends: [jest.configs["flat/recommended"]],
    files: ["tests/**/*.test.ts"],
    rules: {
      // Recognize helpers that assert by throwing on failure
      "jest/expect-expect": [
        "warn",
        { assertFunctionNames: ["expect", "loginAndRent"] },
      ],

      // Not using Jest — this rule tries to detect the Jest package version.
      "jest/no-deprecated-functions": "off",
    },
    settings: {
      // Tells eslint-plugin-jest to recognize describe/it/expect from bun:test
      // instead of assuming the global `jest` object.
      jest: { globalPackage: "bun:test" },
    },
  },

  // RegExp best practices
  regExp.recommended,

  // ESLint comments best practices
  eslintComments.recommended,

  // Override ESLint rules that would conflict with Prettier formatting
  prettier,
]);

export default config;
