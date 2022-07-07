#!/usr/bin/env groovy

def user_id
def user_name
def group_id
def group_name
def container_ccache_dir
def host_ccache_dir

def thread_count = 10

node('wt') {
    user_id = sh(returnStdout: true, script: 'id -u').trim()
    user_name = sh(returnStdout: true, script: 'id -un').trim()
    group_id = sh(returnStdout: true, script: 'id -g').trim()
    group_name = sh(returnStdout: true, script: 'id -gn').trim()
    container_ccache_dir = "/home/${user_name}/.ccache"
    host_ccache_dir = "/local/home/${user_name}/.ccache"
}

def wt_configure(Map args) {
    sh """cmake .. \
            -DCMAKE_C_COMPILER_LAUNCHER=ccache \
            -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
            -DBUILD_EXAMPLES=ON \
            -DBUILD_TESTS=ON \
            -DCONNECTOR_FCGI=ON \
            -DCONNECTOR_HTTP=ON \
            -DDEBUG=ON \
            -DEXAMPLES_CONNECTOR=${args.examplesConnector} \
            -DENABLE_HARU=ON \
            -DENABLE_PANGO=ON \
            -DENABLE_QT5=ON \
            -DENABLE_SQLITE=ON \
            -DUSE_SYSTEM_SQLITE3=ON \
            -DENABLE_POSTGRES=ON \
            -DENABLE_FIREBIRD=ON \
            -DENABLE_MYSQL=ON \
            -DENABLE_MSSQLSERVER=ON \
            -DENABLE_SSL=ON \
            -DHTTP_WITH_ZLIB=ON \
            -DSHARED_LIBS=ON \
            -DBOOST_PREFIX=/opt/boost \
            -DMULTI_THREADED=${args.mt}"""
}

pipeline {
    environment {
        EMAIL = credentials('wt-dev-mail')
    }
    options {
        buildDiscarder logRotator(numToKeepStr: '20')
        disableConcurrentBuilds abortPrevious: true
    }
    agent {
        dockerfile {
            label 'wt'
            dir 'jenkins/linux'
            filename 'full.Dockerfile'
            args "--env CCACHE_DIR=${container_ccache_dir} --env CCACHE_MAXSIZE=20G --volume ${host_ccache_dir}:${container_ccache_dir}:z"
            additionalBuildArgs """--build-arg USER_ID=${user_id} \
                                   --build-arg USER_NAME=${user_name} \
                                   --build-arg GROUP_ID=${group_id} \
                                   --build-arg GROUP_NAME=${group_name} \
                                   --build-arg THREAD_COUNT=${thread_count}"""
        }
    }
    triggers {
        pollSCM('@midnight')
    }
    stages {
        stage('Multi-threaded, wthttp') {
            steps {
                dir('build-mt-http') {
                    wt_configure(mt: 'ON', examplesConnector: 'wthttp')
                    sh "make -k -j${thread_count}"
                    sh "make -C examples -k -j${thread_count}"
                }
                dir('test') {
                    warnError('mt wthttp test.wt failed') {
                        sh "../build-mt-http/test/test.wt --log_format=JUNIT --log_level=all --log_sink=${env.WORKSPACE}/mt_wthttp_test_log.xml"

                    }
                }
            }
        }
        stage('Single-threaded, wthttp') {
            steps {
                dir('build-st-http') {
                    wt_configure(mt: 'OFF', examplesConnector: 'wthttp')
                    sh "make -k -j${thread_count}"
                    sh "make -C examples -k -j${thread_count}"
                }
                dir('test') {
                    warnError('st wthttp test.wt failed') {
                        sh "../build-st-http/test/test.wt --log_format=JUNIT --log_level=all --log_sink=${env.WORKSPACE}/st_wthttp_test_log.xml"

                    }
                }
            }
        }
        stage('Multi-threaded, wtfcgi') {
            steps {
                dir('build-mt-fcgi') {
                    wt_configure(mt: 'ON', examplesConnector: 'wtfcgi')
                    sh "make -k -j${thread_count}"
                    sh "make -C examples -k -j${thread_count}"
                }
                dir('test') {
                    warnError('mt wtfcgi test.wt failed') {
                        sh "../build-mt-fcgi/test/test.wt --log_format=JUNIT --log_level=all --log_sink=${env.WORKSPACE}/mt_wtfcgi_test_log.xml"

                    }
                }
            }
        }
        stage('Single-threaded, wtfcgi') {
            steps {
                dir('build-st-fcgi') {
                    wt_configure(mt: 'OFF', examplesConnector: 'wtfcgi')
                    sh "make -k -j${thread_count}"
                    sh "make -C examples -k -j${thread_count}"
                }
                dir('test') {
                    warnError('st wtfcgi test.wt failed') {
                        sh "../build-st-fcgi/test/test.wt --log_format=JUNIT --log_level=all --log_sink=${env.WORKSPACE}/st_wtfcgi_test_log.xml"

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
        failure {
            mail to: env.EMAIL,
                 subject: "Failed Pipeline: ${currentBuild.fullDisplayName}",
                 body: "Something is wrong with ${env.BUILD_URL}"
        }
        unstable {
            mail to: env.EMAIL,
                 subject: "Unstable Pipeline: ${currentBuild.fullDisplayName}",
                 body: "Something is wrong with ${env.BUILD_URL}"
        }
    }
}
