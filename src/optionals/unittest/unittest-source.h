#define DICTU_UNITTEST_SOURCE "import Inspect;\n" \
"import System;\n" \
"\n" \
"abstract class UnitTest {\n" \
"    var METHOD_NAME_PADDING = '    ';\n" \
"    var RESULTS_PADDING     = '    ';\n" \
"    var ASSERTION_PADDING   = '         ';\n" \
"\n" \
"    init(var onlyFailures = false, var exitOnFailure = false) {\n" \
"        this.results = {\n" \
"            'passed': 0,\n" \
"            'failed': 0,\n" \
"            'skipped': 0\n" \
"        };\n" \
"    }\n" \
"\n" \
"    filterMethods() {\n" \
"        return this.methods().filter(def (method) => {\n" \
"            if (method.startsWith('test') and not method.endsWith('Provider') and not method.endsWith('_skipped')) {\n" \
"                return method;\n" \
"            }\n" \
"\n" \
"            if (method.endsWith('_skipped')) {\n" \
"                this.results['skipped'] += 1;\n" \
"            }\n" \
"        });\n" \
"    }\n" \
"\n" \
"    setUp() {\n" \
"\n" \
"    }\n" \
"\n" \
"    tearDown() {\n" \
"\n" \
"    }\n" \
"\n" \
"    run() {\n" \
"        const methods = this.filterMethods();\n" \
"\n" \
"        print(Inspect.getFile(1));\n" \
"        methods.forEach(def (method) => {\n" \
"            print('{}{}()'.format(UnitTest.METHOD_NAME_PADDING, method));\n" \
"\n" \
"            const providerMethodName = '{}Provider'.format(method);\n" \
"\n" \
"            if (this.hasAttribute(providerMethodName)) {\n" \
"                const testValue = this.getAttribute(providerMethodName)();\n" \
"\n" \
"                if (type(testValue) == 'list') {\n" \
"                    testValue.forEach(def (val) => {\n" \
"                        this.setUp();\n" \
"                        this.getAttribute(method)(val);\n" \
"                        this.tearDown();\n" \
"                    });\n" \
"                } else {\n" \
"                    this.setUp();\n" \
"                    this.getAttribute(method)(testValue);\n" \
"                    this.tearDown();\n" \
"                }\n" \
"            } else {\n" \
"                this.setUp();\n" \
"                this.getAttribute(method)();\n" \
"                this.tearDown();\n" \
"            }\n" \
"        });\n" \
"        print('\nResults:\n{}- {} assertion(s) were successful.\n{}- {} assertion(s) were failures.\n{}- {} method(s) were skipped.\n'.format(\n" \
"            UnitTest.RESULTS_PADDING,\n" \
"            this.results['passed'],\n" \
"            UnitTest.RESULTS_PADDING,\n" \
"            this.results['failed'],\n" \
"            UnitTest.RESULTS_PADDING,\n" \
"            this.results['skipped']\n" \
"        ));\n" \
"\n" \
"        if (this.results['failed'] > 0) {\n" \
"            System.exit(1);\n" \
"        }\n" \
"    }\n" \
"\n" \
"    printResult(success, errorMsg) {\n" \
"        if (success) {\n" \
"            this.results['passed'] += 1;\n" \
"\n" \
"            if (not this.onlyFailures) {\n" \
"                print('{}Success.'.format(UnitTest.ASSERTION_PADDING));\n" \
"            }\n" \
"        } else {\n" \
"            this.results['failed'] += 1;\n" \
"\n" \
"            print('{}Line: {} - {}'.format(UnitTest.ASSERTION_PADDING, Inspect.getLine(2), errorMsg));\n" \
"\n" \
"            if (this.exitOnFailure) {\n" \
"                System.exit(1);\n" \
"            }\n" \
"        }\n" \
"    }\n" \
"\n" \
"    assertEquals(value, expected) {\n" \
"        this.printResult(value == expected, 'Failure: {} is not equal to {}.'.format(value, expected));\n" \
"    }\n" \
"\n" \
"    assertTruthy(value) {\n" \
"        this.printResult(value, 'Failure: {} is not Truthy.'.format(value));\n" \
"    }\n" \
"\n" \
"    assertFalsey(value) {\n" \
"        this.printResult(not value, 'Failure: {} is not Falsey.'.format(value));\n" \
"    }\n" \
"\n" \
"    assertSuccess(value) {\n" \
"        if (type(value) != 'result') {\n" \
"            this.printResult(false, 'Failure: {} is not a Result type.'.format(value));\n" \
"            return;\n" \
"        }\n" \
"\n" \
"        this.printResult(value.success(), 'Failure: {} is not a Result type in a success state.'.format(value));\n" \
"    }\n" \
"\n" \
"    assertError(value) {\n" \
"        if (type(value) != 'result') {\n" \
"            this.printResult(false, 'Failure: {} is not a Result type.'.format(value));\n" \
"            return;\n" \
"        }\n" \
"\n" \
"        this.printResult(not value.success(), 'Failure: {} is not a Result type in an error state.'.format(value));\n" \
"    }\n" \
"}\n" \

