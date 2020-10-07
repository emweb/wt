properties([
    pipelineTriggers([
        pollSCM('@daily')
    ])
])

def user_id
def user_name
def group_id
def group_name

def thread_count = 5

node('docker') {
    user_id = sh(returnStdout: true, script: 'id -u').trim()
    user_name = sh(returnStdout: true, script: 'id -un').trim()
    group_id = sh(returnStdout: true, script: 'id -g').trim()
    group_name = sh(returnStdout: true, script: 'id -gn').trim()
}

def wt_configure(Map args) {
    sh """cmake .. \
            -DBUILD_EXAMPLES=ON \
            -DBUILD_TESTS=ON \
            -DCONNECTOR_FCGI=OFF \
            -DCONNECTOR_HTTP=ON \
            -DEXAMPLES_CONNECTOR=wthttp \
            -DENABLE_HARU=OFF \
            -DENABLE_PANGO=OFF \
            -DENABLE_QT4=OFF \
            -DENABLE_QT5=OFF \
            -DENABLE_SQLITE=ON \
            -DUSE_SYSTEM_SQLITE3=ON \
            -DENABLE_POSTGRES=ON \
            -DENABLE_FIREBIRD=OFF \
            -DENABLE_MYSQL=ON \
            -DENABLE_MSSQLSERVER=ON \
            -DENABLE_SSL=OFF \
            -DHTTP_WITH_ZLIB=OFF \
            -DSHARED_LIBS=${args.shared} \
            -DMULTI_THREADED=ON"""
}

def branch(Map args) {
    def builddir = "build-shared-${args.shared}"
    stage("Build shared=${args.shared}") {
        args.image.inside() {
            dir(builddir) {
                wt_configure(shared: args.shared)
                sh "make -k -j${args.thread_count}"
            }
        }
    }
    stage("Test Wt shared=${args.shared}") {
        args.image.inside() {
            dir('test') {
                warnError('test.wt failed') {
                    sh "../${builddir}/test/test.wt --report_level=detailed --log_format=JUNIT --log_level=all --log_sink=${env.WORKSPACE}/shared_${args.shared}_test_log.xml"
                }
            }
        }
    }
    stage("Test SQLite3 shared=${args.shared}") {
        args.image.inside() {
            dir('test') {
                warnError('test.sqlite3 failed') {
                    sh "../${builddir}/test/test.sqlite3 --report_level=detailed --log_format=JUNIT --log_level=all --log_sink=${env.WORKSPACE}/shared_${args.shared}_sqlite3_test_log.xml"
                }
            }
        }
    }
    stage("Test Postgres shared=${args.shared}") {
        docker.image('postgres').withRun('-e POSTGRES_PASSWORD=postgres_test -e POSTGRES_USER=postgres_test -e POSTGRES_DB=wt_test') { c ->
            sleep 10
            args.image.inside("--link ${c.id}:db") {
                dir('test') {
                    warnError('test.postgres failed') {
                        sh "../${builddir}/test/test.postgres --report_level=detailed --log_format=JUNIT --log_level=all --log_sink=${env.WORKSPACE}/shared_${args.shared}_postgres_test_log.xml"
                    }
                }
            }
        }
    }
    stage("Test SQL Server shared=${args.shared}") {
        docker.image('mcr.microsoft.com/mssql/server:2019-latest').withRun('-e ACCEPT_EULA=Y -e \'SA_PASSWORD=hereIsMyPassword_1234\' -e MSSQL_PID=Express') { c ->
            sleep 60
            args.image.inside("--link ${c.id}:db") {
                dir('test') {
                    warnError('test.mssqlserver failed') {
                        sh "../${builddir}/test/test.mssqlserver --report_level=detailed --log_format=JUNIT --log_level=all --log_sink=${env.WORKSPACE}/shared_${args.shared}_mssqlserver_test_log.xml"
                    }
                }
            }
        }
    }
    stage("Test MySQL/MariaDB shared=${args.shared}") {
        docker.image('mariadb').withRun('-e MYSQL_ROOT_PASSWORD=test_pw -e MYSQL_DATABASE=wt_test_db -e MYSQL_USER=test_user -e MYSQL_PASSWORD=test_pw') { c ->
            sleep 40
            args.image.inside("--link ${c.id}:db") {
                dir('test') {
                    warnError('test.mysql failed') {
                        sh "../${builddir}/test/test.mysql --report_level=detailed --log_format=JUNIT --log_level=all --log_sink=${env.WORKSPACE}/shared_${args.shared}_mysql_test_log.xml"
                    }
                }
            }
        }
    }
}

node('docker') {
    try {
        stage('Checkout') {
            git branch: "${env.BRANCH_NAME}",
                url: 'http://vierwerf/git/wt/.git'
        }
        def image = docker.build("wt-full:${env.BUILD_ID}",
                                 """./jenkins \
                                    -f ./jenkins/full.Dockerfile \
                                    --build-arg USER_ID=${user_id} \
                                    --build-arg USER_NAME=${user_name} \
                                    --build-arg GROUP_ID=${group_id} \
                                    --build-arg GROUP_NAME=${group_name} \
                                    --build-arg THREAD_COUNT=${thread_count}""")
        branch shared: 'ON', image: image, thread_count: thread_count
        branch shared: 'OFF', image: image, thread_count: thread_count
    } finally {
        // JUnit:
        junit '*_test_log.xml'
        // Cleanup:
        sh 'docker volume prune -f'
        cleanWs()
        def lastBuildStatus = currentBuild.previousBuild?.result ?: 'SUCCESS'
        // Mail on error:
        withCredentials([string(credentialsId: 'wt-dev-mail', variable: 'EMAIL')]) {
            if (currentBuild.currentResult != 'SUCCESS' &&
                lastBuildStatus == 'SUCCESS') {
                mail to: env.EMAIL,
                     subject: "Failed Pipeline: ${currentBuild.fullDisplayName}",
                     body: "Something is wrong with ${env.BUILD_URL}"
            }
            if (currentBuild.currentResult == 'SUCCESS' &&
                lastBuildStatus != 'SUCCESS') {
                mail to: env.EMAIL,
                     subject: "Fixed Pipeline: ${currentBuild.fullDisplayName}",
                     body: "Build ${env.BUILD_URL} is OK"
            }
        }
    }
}
