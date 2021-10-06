#define DICTU_UNITTEST_SOURCE "abstract class UnitTest {\n" \
"    var METHOD_NAME_PADDING = '    ';\n" \
"    var RESULTS_PADDING     = '    ';\n" \
"    var ASSERTION_PADDING   = '         ';\n" \
"    var results = {\n" \
"        'passed': 0,\n" \
"        'failed': 0,\n" \
"        'skipped': 0\n" \
"    };\n" \
"\n" \
"    init(private onlyFailures = false) {}\n" \
"\n" \
"    filterMethods() {\n" \
"        return this.methods().filter(def (method) => {\n" \
"            if (method.startsWith('test') and !method.endsWith('Provider') and !method.endsWith('_skipped')) {\n" \
"\n" \
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
"        print(__file__);\n" \
"        methods.forEach(def (method) => {\n" \
"            print('{}{}()'.format(UnitTest.METHOD_NAME_PADDING, method));\n" \
"            this.setUp();\n" \
"\n" \
"            const providerMethodName = '{}Provider'.format(method);\n" \
"\n" \
"            if (this.hasAttribute(providerMethodName)) {\n" \
"                const testValue = this.getAttribute(providerMethodName)();\n" \
"\n" \
"                if (type(testValue) == 'list') {\n" \
"                    testValue.forEach(def (val) => {\n" \
"                        this.getAttribute(method)(val);\n" \
"                    });\n" \
"                } else {\n" \
"                    this.getAttribute(method)(testValue);\n" \
"                }\n" \
"            } else {\n" \
"                this.getAttribute(method)();\n" \
"            }\n" \
"\n" \
"\n" \
"            this.tearDown();\n" \
"        });\n" \
"        print('\nResults:\n{}- {} assertion(s) were successful.\n{}- {} assertion(s) were failures.\n{}- {} method(s) were skipped.\n'.format(\n" \
"            UnitTest.RESULTS_PADDING,\n" \
"            this.results['passed'],\n" \
"            UnitTest.RESULTS_PADDING,\n" \
"            this.results['failed'],\n" \
"            UnitTest.RESULTS_PADDING,\n" \
"            this.results['skipped']\n" \
"        ));\n" \
"    }\n" \
"\n" \
"    printResult(success, errorMsg) {\n" \
"        if (success) {\n" \
"            this.results['passed'] += 1;\n" \
"\n" \
"            if (!this.onlyFailures) {\n" \
"                print('{}Success.'.format(UnitTest.ASSERTION_PADDING));\n" \
"            }\n" \
"        } else {\n" \
"            this.results['failed'] += 1;\n" \
"\n" \
"            print('{}{}'.format(UnitTest.ASSERTION_PADDING, errorMsg));\n" \
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
"        this.printResult(!value, 'Failure: {} is not Falsey.'.format(value));\n" \
"    }\n" \
"\n" \
"    assertSuccess(value) {\n" \
"\n" \
"    }\n" \
"\n" \
"    assertError(value) {\n" \
"\n" \
"    }\n" \
"}\n" \

