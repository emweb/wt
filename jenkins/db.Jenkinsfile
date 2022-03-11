properties([
    pipelineTriggers([
        pollSCM('@midnight')
    ])
])

def user_id
def user_name
def group_id
def group_name

def thread_count = 5

node('wt') {
    user_id = sh(returnStdout: true, script: 'id -u').trim()
    user_name = sh(returnStdout: true, script: 'id -un').trim()
    group_id = sh(returnStdout: true, script: 'id -g').trim()
    group_name = sh(returnStdout: true, script: 'id -gn').trim()
}

def wt_configure(Map args) {
    sh """cmake .. \
            -DCMAKE_C_COMPILER_LAUNCHER=ccache \
            -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
            -DBUILD_EXAMPLES=ON \
            -DBUILD_TESTS=ON \
            -DCONNECTOR_FCGI=OFF \
            -DCONNECTOR_HTTP=ON \
            -DDEBUG=ON \
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
    def ccache_args = "--env CCACHE_DIR=${args.container_ccache_dir} --env CCACHE_MAXSIZE=20G --volume ${args.host_ccache_dir}:${args.container_ccache_dir}:z"
    stage("Build shared=${args.shared}") {
        args.image.inside(ccache_args) {
            dir(builddir) {
                wt_configure(shared: args.shared)
                sh "make -k -j${args.thread_count}"
            }
        }
    }
    stage("Test Wt shared=${args.shared}") {
        args.image.inside(ccache_args) {
            dir('test') {
                warnError('test.wt failed') {
                    sh "../${builddir}/test/test.wt --report_level=detailed --log_format=JUNIT --log_level=all --log_sink=${env.WORKSPACE}/shared_${args.shared}_test_log.xml"
                }
            }
        }
    }
    stage("Test SQLite3 shared=${args.shared}") {
        args.image.inside(ccache_args) {
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
            args.image.inside("--link ${c.id}:db ${ccache_args}") {
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
            args.image.inside("--link ${c.id}:db ${ccache_args}") {
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
            args.image.inside("--link ${c.id}:db ${ccache_args}") {
                dir('test') {
                    warnError('test.mysql failed') {
                        sh "../${builddir}/test/test.mysql --report_level=detailed --log_format=JUNIT --log_level=all --log_sink=${env.WORKSPACE}/shared_${args.shared}_mysql_test_log.xml"
                    }
                }
            }
        }
    }
}

node('wt') {
    try {
        stage('Checkout') {
            checkout scm
        }
        def image = docker.build("wt-full:${env.BRANCH_NAME}-${env.BUILD_ID}",
                                 """./jenkins \
                                    -f ./jenkins/full.Dockerfile \
                                    --build-arg USER_ID=${user_id} \
                                    --build-arg USER_NAME=${user_name} \
                                    --build-arg GROUP_ID=${group_id} \
                                    --build-arg GROUP_NAME=${group_name} \
                                    --build-arg THREAD_COUNT=${thread_count}""")
        def container_ccache_dir = "/home/${user_name}/.ccache"
        def host_ccache_dir = "/local/home/${user_name}/.ccache"
        branch shared: 'ON', image: image, thread_count: thread_count, container_ccache_dir: container_ccache_dir, host_ccache_dir: host_ccache_dir
        branch shared: 'OFF', image: image, thread_count: thread_count, container_ccache_dir: container_ccache_dir, host_ccache_dir: host_ccache_dir
    } finally {
        // JUnit:
        junit '*_test_log.xml'
        // Cleanup:
        sh 'docker volume prune -f'
        cleanWs()
        // Mail on error:
        withCredentials([string(credentialsId: 'wt-dev-mail', variable: 'EMAIL')]) {
            if (currentBuild.currentResult == 'FAILURE') {
                mail to: env.EMAIL,
                     subject: "Failed Pipeline: ${currentBuild.fullDisplayName}",
                     body: "Something is wrong with ${env.BUILD_URL}"
            }
            if (currentBuild.currentResult == 'UNSTABLE') {
                mail to: env.EMAIL,
                     subject: "Unstable Pipeline: ${currentBuild.fullDisplayName}",
                     body: "Something is wrong with ${env.BUILD_URL}"
            }
        }
    }
}
