def thread_count = 1

def wt_configure(Map args) {
    sh """cmake .. \
            -DBUILD_EXAMPLES=ON \
            -DBUILD_TESTS=ON \
            -DCONNECTOR_FCGI=OFF \
            -DCONNECTOR_HTTP=ON \
            -DEXAMPLES_CONNECTOR=wthttp \
            -DENABLE_HARU=OFF \
            -DENABLE_PANGO=OFF \
            -DENABLE_POSTGRES=OFF \
            -DENABLE_QT4=OFF \
            -DENABLE_QT5=OFF \
            -DENABLE_SQLITE=ON \
            -DENABLE_SSL=OFF \
            -DHTTP_WITH_ZLIB=OFF \
            -DSHARED_LIBS=OFF \
            -DMULTI_THREADED=${args.mt}"""
}

pipeline {
    environment {
        EMAIL = credentials('wt-dev-mail')
    }
    options {
        buildDiscarder logRotator(numToKeepStr: '20')
        disableConcurrentBuilds()
    }
    agent {
        label 'build-freebsd12-1'
    }
    triggers {
        pollSCM('H/5 * * * *')
    }
    stages {
        stage('Single-threaded') {
            steps {
                dir('build-st') {
                    wt_configure(mt: 'OFF')
                    sh "make -k -j${thread_count}"
                    sh "make -C examples -k -j${thread_count}"
                }
                dir('test') {
                    warnError('non-mt test.wt failed') {
                        sh "../build-st/test/test.wt --log_format=JUNIT --log_level=all --log_sink=${env.WORKSPACE}/st_test_log.xml"
                    }
                }
            }
        }
        stage('Multithreaded') {
            steps {
                dir('build-mt') {
                    wt_configure(mt: 'ON')
                    sh "make -k -j${thread_count}"
                    sh "make -C examples -k -j${thread_count}"
                }
                dir('test') {
                    warnError('mt test.wt failed') {
                        sh "../build-mt/test/test.wt --log_format=JUNIT --log_level=all --log_sink=${env.WORKSPACE}/mt_test_log.xml"
                    }
                }
            }
        }
    }
    post {
        always {
            junit '*_test_log.xml'
        }
        cleanup {
            cleanWs()
        }
        regression {
            mail to: env.EMAIL,
                 subject: "Failed Pipeline: ${currentBuild.fullDisplayName}",
                 body: "Something is wrong with ${env.BUILD_URL}"
        }
        fixed {
            mail to: env.EMAIL,
                 subject: "Failed Pipeline: ${currentBuild.fullDisplayName}",
                 body: "Build ${env.BUILD_URL} is OK"
        }
    }
}
