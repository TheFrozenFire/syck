#
# YAML::YamlNode class
#
require 'yaml/ypath'

module YAML

    #
    # YAML Generic Model container
    #
    class YamlNode
        attr_accessor :kind, :type_id, :value, :anchor
        def initialize( t, v )
            @type_id = t
            if Hash === v
                @kind = 'map'
                @value = {}
                v.each { |k,v|
                    @value[ k.transform ] = [ k, v ]
                }
            elsif Array === v
                @kind = 'seq'
                @value = v
            elsif String === v
                @kind = 'scalar'
                @value = v
            end
        end

        #
        # Search for YPath entry and return
        # qualified nodes.
        #
        def select( ypath_str )
            matches = match_path( ypath_str )

            #
            # Create a new generic view of the elements selected
            #
            if matches
                result = []
                matches.each { |m|
                    result.push m.last
                }
                YamlNode.new( 'seq', result )
            end
        end

        #
        # Search for YPath entry and return
        # transformed nodes.
        #
        def select!( ypath_str )
            matches = match_path( ypath_str )

            #
            # Create a new generic view of the elements selected
            #
            if matches
                result = []
                matches.each { |m|
                    result.push m.last.transform
                }
                result
            end
        end

        #
        # Search for YPath entry and return a list of
        # qualified paths.
        #
        def search( ypath_str )
            matches = match_path( ypath_str )

            if matches
                matches.collect { |m|
                    path = []
                    m.each_index { |i|
                        path.push m[i] if ( i % 2 ).zero?
                    }
                    "/" + path.compact.join( "/" )
                }
            end
        end

        def at( seg )
            if Hash === @value and @value.has_key?( seg )
                @value[seg][1]
            elsif Array === @value and seg =~ /\A\d+\Z/ and @value[seg.to_i]
                @value[seg.to_i]
            end
        end

        #
        # YPath search returning a complete depth array
        #
        def match_path( ypath_str )
            depth = 0
            matches = []
            YPath.each_path( ypath_str ) do |ypath|
                seg = match_segment( ypath, 0 )
                matches += seg if seg
            end
            matches.uniq
        end

        #
        # Search a node for a single YPath segment
        #
        def match_segment( ypath, depth )
            deep_nodes = []
            seg = ypath.segments[ depth ]
            if seg == "/"
                unless String === @value
                    idx = -1
                    @value.collect { |v|
                        idx += 1
                        if Hash === @value
                            match_init = [v[0], v[1][1]]
                            match_deep = v[1][1].match_segment( ypath, depth )
                        else
                            match_init = [idx, v]
                            match_deep = v.match_segment( ypath, depth )
                        end
                        if match_deep
                            match_deep.each { |m|
                                deep_nodes.push( match_init + m )
                            }
                        end
                    }
                end
                depth += 1
                seg = ypath.segments[ depth ]
            end
            match_nodes =
                case seg
                when "."
                    [[nil, self]]
                when ".."
                    [["..", nil]]
                when "*"
                    if @value.is_a? Enumerable
                        idx = -1
                        @value.collect { |h|
                            idx += 1
                            if Hash === @value
                                [h[0], h[1][1]]
                            else
                                [idx, h]
                            end
                        }
                    end
                else
                    if seg =~ /^"(.*)"$/
                        seg = $1
                    elsif seg =~ /^'(.*)'$/
                        seg = $1
                    end
                    if ( v = at( seg ) )
                        [[ seg, v ]]
                    end
                end
            return deep_nodes unless match_nodes
            pred = ypath.predicates[ depth ]
            if pred
                case pred
                when /^\.=/
                    pred = $'
                    match_nodes.reject! { |n|
                        n.last.value != pred
                    }
                else
                    match_nodes.reject! { |n|
                        n.last.at( pred ).nil?
                    }
                end
            end
            return match_nodes + deep_nodes unless ypath.segments.length > depth + 1

            #puts "DEPTH: #{depth + 1}"
            deep_nodes = []
            match_nodes.each { |n|
                if n[1].is_a? YamlNode
                    match_deep = n[1].match_segment( ypath, depth + 1 )
                    if match_deep
                        match_deep.each { |m|
                            deep_nodes.push( n + m )
                        }
                    end
                else
                    deep_nodes = []
                end
            }
            deep_nodes = nil if deep_nodes.length == 0
            deep_nodes
        end

        #
        # Transform this node fully into a native type
        #
        def transform
            t = nil
            if @value.is_a? Hash
                t = {}
                @value.each { |k,v|
                    t[ k ] = v[1].transform
                }
            elsif @value.is_a? Array
                t = []
                @value.each { |v|
                    t.push v.transform
                }
            else
                t = @value
            end
            YAML.transfer_method( @type_id, t )
        end

        #
        # We want the node to act like as Hash
        # if it is.
        #
        def []( *k )
            if Hash === @value
                v = @value.[]( *k )
                v[1] if v
            elsif Array === @value
                @value.[]( *k )
            end
        end

        def children
            if Hash === @value
                @value.values.collect { |c| c[1] }
            elsif Array === @value
                @value
            end
        end

        def children_with_index
            if Hash === @value
                @value.keys.collect { |i| [self[i], i] }
            elsif Array === @value
                i = -1; @value.collect { |v| i += 1; [v, i] }
            end
        end

        def emit
            transform.to_yaml
        end
    end

end
